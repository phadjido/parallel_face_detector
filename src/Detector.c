#include <math.h>

#include "tanh_lup.h"
#include "NN.h"
#include "Convolver.h"
#include "FastConv.h"
#include "ConvolverFine.h"

#include "LpiImage.h"

#include <sys/types.h>
#include <sys/time.h>

double my_gettime()
{
  struct timeval tv;
  gettimeofday(&tv, 0);
  return (double) (tv.tv_sec+tv.tv_usec/1000000.0);
}

//draws a red pixel in a three-channel colour image
#define PIX(y, x) ip[((y)*width+(x))*3]=(char)0;ip[((y)*width+(x))*3+1]=(char)0;ip[((y)*width+(x))*3+2]=(char)255;
#define MAX_FACES 1000

#define THRESHOLD 1.0


//CNN WIDTH _ HEIGHT
int SH = 36;
int SW = 32;
float SR = 36.0/32.0; // ((float)SW)/((float)SH);


//GLOBAL VARIABLES
struct CNN gl_cnn;


//STATIC VARIABLES
/* indirect references have to be improved, e.g. data->x[i] => x = data->x; x[i]... */
struct per_image_data {
	float*** outputValues;
	struct lpiImage **imgt;
	struct lpiImage **imgt_f;
	struct CConvolver *cconv;

	char gl_FileTitle[256];


	//contain position, scale and score for each target detected after convolve
	int *Xs;
	int *Ys;
	float *outputs;
	int *heights;

	//contain position, scale and score for each cluster center
	int *Xf;
	int *Yf;
	float *outf;
	int *Hf;


	int gl_S1;
	int gl_S2;
	double gl_ST;
	int gl_height;
	int gl_width;

	int allX[MAX_FACES];
	int allY[MAX_FACES];
	int allH[MAX_FACES];
	float allOut[MAX_FACES];

	int vg_deleted[MAX_FACES];
	int discarded[MAX_FACES];
};

void InitData(struct per_image_data *data)
{
	data->outputValues = NULL;
	data->imgt = NULL;
	data->imgt_f = NULL;
	data->cconv = NULL;


	//contain position, scale and score for each target detected after convolve
	data->Xs = NULL;
	data->Ys = NULL;
	data->outputs = NULL;
	data->heights = NULL;

	//contain position, scale and score for each cluster center
	data->Xf = NULL;
	data->Yf = NULL;
	data->outf = NULL;
	data->Hf = NULL;


	data->gl_S1 = 0;
	data->gl_S2 = 0;
	data->gl_ST = 0.0;
}


//STATIC FUNCTIONS
int searchFacesConv(struct CNN *cnn, struct lpiImage *img_source, int S1, int S2, double ST);
int searchFacesConvFine(struct CNN *cnn, struct lpiImage *img_source, int S1, int S2, double ST,
	int *X, int *Y, int *H, int *W, float *O);
int searchFacesConvFineStatic(struct per_image_data *data, struct CNN *cnn, struct lpiImage *img_source, int S1, int S2, double ST,
	int *X, int *Y, int *H, int *W, float *O);

void drawFace(struct lpiImage *img, int CX, int CY, int CW, int CH, float vol, int size, struct lpiImage *img_input, int id);
void savePGM(struct lpiImage *img, int count);
void VoteGrouping2(struct per_image_data *data, int *Xs, int *Ys, float *outputs, int *heights, int size,
	double minVoteSize, double thrMaxVote, int *Xf, int *Yf, float *outf, int *Hf, int *nb);
int overlap(int i, int j, int Xri, int Yri, int Hri, int Wri, float outri,
		int Xrj, int Yrj, int Hrj, int Wrj, float outrj, float *perc);


int InitImages(struct per_image_data *data, int width, int height, int S1, int S2, double ST, int still_image);
void FreeImages(struct per_image_data *data, int still_image);

int my_round(double x);
int CropRescaleConvolve(struct CNN *cnn, struct lpiImage *img, float S, int XoS, int YoS,
	float *maxOut, int *Xo, int *Yo, float *So, float *mean);

int CropRescaleConvolveFloat(struct CNN *cnn, struct lpiImage *img, float S, int XoS, int YoS,
	float *maxOut, int *Xo, int *Yo, float *So, float *mean);


extern int gl_playcam;


static int videoInit=0;

//////////////////////////

void InitCFF()
{
	CNN_CreateConvolveKernels(&gl_cnn);
	init_tanh();
}

int callbackStillImage(struct lpiImage* image, int *X, int *Y, int *H, int *W, float *O)
{
	struct per_image_data data;
	int nofFaces;

	printf("Processing image %s\n", image->name);
	InitData(&data);
	InitImages(&data, image->width, image->height, 36, 360, 1.2, 1);
	nofFaces=searchFacesConvFineStatic(&data, &gl_cnn, image, 36, 360, 1.2, X, Y, H, W, O);
	FreeImages(&data, 1);
	return(nofFaces);
}

int InitImages(struct per_image_data *data, int width, int height, int S1, int S2, double ST, int still_image)
{

	int S;
	//find all the different scales to try
	int nbS=0;
	for(S=S1; S<=S2; S=(int)floor((float)(S)*ST))
	{
		int h1 = (int)floor((double)height*((double)SH/(double)S));
		int w1 = (int)floor((double)width*((double)SH/(double)S));

		if(h1>=36 && w1>=32)
			nbS++;
	}
	data->imgt = (struct lpiImage **)calloc(nbS, sizeof(struct lpiImage *));
	data->cconv = (struct CConvolver *)calloc(nbS, sizeof(struct CConvolver));
	data->imgt_f = (struct lpiImage **)calloc(nbS, sizeof(struct lpiImage *));


	nbS=0;
	for(S=S1; S<=S2; S=(int)floor((float)(S)*ST))
	{
		int h1 = (int)floor((double)height*((double)SH/(double)S));
		int w1 = (int)floor((double)width*((double)SH/(double)S));

		if(h1<36 || w1<32)
			continue;

		if(still_image==0)
		{
			data->imgt[nbS]=lpiImage_lpiCreateImage(w1, h1, sizeof(unsigned char));
			data->imgt_f[nbS]=lpiImage_lpiCreateImage(w1, h1, sizeof(float));
			CConvolver_InitFMs(&data->cconv[nbS], w1, h1);
		}
		CConvolver_SetCNN(&data->cconv[nbS], &gl_cnn);
		nbS++;
	}

	data->Xs=Alloc1DInt(MAX_FACES);
	data->Ys=Alloc1DInt(MAX_FACES);
	data->outputs=Alloc1DFloat(MAX_FACES);
	data->heights=Alloc1DInt(MAX_FACES);

	data->Xf=Alloc1DInt(MAX_FACES);
	data->Yf=Alloc1DInt(MAX_FACES);
	data->outf=Alloc1DFloat(MAX_FACES);
	data->Hf=Alloc1DInt(MAX_FACES);

	data->gl_S1=S1;
	data->gl_S2=S2;
	data->gl_ST=ST;
	data->gl_width=width;
	data->gl_height=height;

	return(1);
}


void FreeImages(struct per_image_data *data, int still_image)
{
	int S;
	int nbS=0;
	for(S=data->gl_S1; S<=data->gl_S2; S=(int)floor((float)(S)*data->gl_ST))
	{
		int h1 = (int)floor((double)data->gl_height*((double)SH/(double)S));
		int w1 = (int)floor((double)data->gl_width*((double)SH/(double)S));

		if(h1<36 || w1<32)
			continue;

		if(still_image==0)
		{
			lpiImage_lpiReleaseImage(&data->imgt[nbS]);
			CConvolver_FreeFMs(&data->cconv[nbS]);
			lpiImage_lpiReleaseImage(&data->imgt_f[nbS]);
		}

		nbS++;
	}
	free(data->imgt);
	free(data->cconv);
	free(data->imgt_f);

	Del1D(data->Xs);
	Del1D(data->Ys);
	Del1D(data->outputs);
	Del1D(data->heights);

	Del1D(data->Xf);
	Del1D(data->Yf);
	Del1D(data->outf);
	Del1D(data->Hf);

	videoInit=0;
}



int CropRescaleConvolve(struct CNN *cnn, struct lpiImage *img, float S, int XoS, int YoS,
	float *maxOut, int *Xo, int *Yo, float *So, float *mean)
{

		//calculate the window portion of the image to crop
		//the top-left center of the area search in the real image is:
		//->displacement for (Xo, Yo)
	float dispCenterY = ((float)8.0)*S/(float)SH;
	float dispCenterX = ((float)8.0)*S/(float)SH;

		//include in the displacemt the (w, h)/2 for putting in the croped
		///image all the stimulation windows
	float dispX=dispCenterX + (S/((float)2.0)) * ((float)SW)/((float)SH);
	float dispY=dispCenterY + S/((float)2.0);


	int topLeftX = XoS - (int)floor((double)dispX);
	int topLeftY = YoS - (int)floor((double)dispY);
	int bottomRightX = XoS + (int)ceil((double)dispX);
	int bottomRightY = YoS + (int)ceil((double)dispY);

	int i, j;
	int ww=bottomRightX-topLeftX+1;
	int hh=bottomRightY-topLeftY+1;


//	lpiImage *imgCr=cvCreateImage( cvSize(ww, hh), IPL_DEPTH_8U, 1);
	struct lpiImage *imgCr=lpiImage_lpiCreateImage(ww, hh, sizeof(unsigned char));

	unsigned char *pimg=(unsigned char *)img->imageData;
	unsigned char *pcr=(unsigned char *)imgCr->imageData;
	int wS=img->width;
	int wwS=imgCr->width;

	int scaledWidth;
	int scaledHeight;

	struct lpiImage *imgScaled;
	float *res;

	struct lpiImage *img_tmp;
	unsigned char *p1;
	float *p2;

	struct CConvolverFine cc;

	int nofPos;

	for(i=0;i<hh;i++) {
		int i_wwS = i*wwS;
		int itopLeftY_wS = (i+topLeftY)*wS;
		for(j=0;j<ww;j++)
		{
			if( (i+topLeftY)<0 || (i+topLeftY)>=img->height || (j+topLeftX)<0 || (j+topLeftX)>=img->width )
				pcr[i_wwS+j]=0;
			else
				pcr[i_wwS+j]=pimg[itopLeftY_wS+j+topLeftX];
		}
	}

	scaledWidth=SW+16;
	scaledHeight=SH+16;

	imgScaled=lpiImage_lpiCreateImage(scaledWidth, scaledHeight, sizeof(unsigned char));

	lpiImage_lpiResize(imgCr, imgScaled);

	res=(float *)calloc(scaledWidth*scaledHeight, sizeof(float));

	img_tmp=lpiImage_lpiCreateImage(scaledWidth, scaledHeight, sizeof(float));
	p1=(unsigned char *)imgScaled->imageData;
	p2=(float *)img_tmp->imageData;

	for(i=0;i<scaledHeight;i++)
		for(j=0;j<scaledWidth;j++)
			p2[i*scaledWidth+j]=(float)( ((int)p1[i*imgScaled->width+j]) - 128)/((float)128.0);

	lpiImage_lpiReleaseImage(&imgCr);
	lpiImage_lpiReleaseImage(&imgScaled);

	CConvolverFine_SetCNN(&cc, cnn);

	CConvolverFine_ConvolveFine(&cc, img_tmp, res);

	nofPos=0;
	for(i=0;i<scaledHeight-SH;i++)
		for(j=0;j<scaledWidth-SW;j++)
		{
			int centerY;
			int centerX;
			int unScaledCenterX;
			int unScaledCenterY;

				//NOTE: thr -> 0.0 !!!!!!!!!!!!!!
			float resf=res[i*scaledWidth+j];
			if(resf<0.0)
				continue;

			nofPos++;
			*mean += resf;


			if(resf < *maxOut)
				continue;


			*maxOut=resf;
				//find the center in the original image
				//--> undo all the transformations we have done
			centerY=i+SH/2;
			centerX=j+SW/2;
			unScaledCenterX=my_round( ((double)centerX) * ((double)S) / ((double)SH) );
			unScaledCenterY=my_round( ((double)centerY) * ((double)S) / ((double)SH) );
			*Xo = unScaledCenterX + topLeftX;
			*Yo = unScaledCenterY + topLeftY;
			*So = S;
		}

	lpiImage_lpiReleaseImage(&img_tmp);


	free(res);

	return(nofPos);
}







void drawFace(struct lpiImage *img, int CX, int CY, int CW, int CH, float vol, int size, struct lpiImage *img_input, int id)
{
//	printf("drawFace: %d %d %d %d\n", CX, CY, CW, CH);
/*
	cvRectangle(img, cvPoint(CX-CW/2, CY-CH/2), cvPoint(CX+CW/2, CY+CH/2), CV_RGB(255, 0, 0), 1);
	CvFont font;
	cvInitFont( &font, CV_FONT_VECTOR0, 0.5, 0.5, 0, 1);
	char buf[32];
	sprintf(buf, "%.3f", vol);
	cvPutText(img, buf, cvPoint(CX-20, CY), &font, CV_RGB(0, 0, 255) );
*/
}




void VoteGrouping2(struct per_image_data *data, int *Xs, int *Ys, float *outputs, int *heights, int size,
	double minVoteSize, double thrMaxVote, int *Xf, int *Yf, float *outf, int *Hf, int *nb)
{
	int i, j;

		//the values for the current cluster center
	float cxNum=0.0;
	float cyNum=0.0;
	float chNum=0.0;
	float Denom=0.0;

	float totalOutput=0.0;

	int CX=0, CY=0, CH=0, CW=0;
	int votesInCluster=0;


	float maxout;
	float swapf;
	int swapi;
	int indmax=0;

	int pointsLeft;

	for(i=0;i<size;i++)
		data->vg_deleted[i]=0;

		//sort points according to ouputvalues
	for(i=0;i<size;i++)
	{
		maxout = outputs[i];
		indmax = i;
	  	for(j=i;j<size;j++)
		{
			if (outputs[j]>maxout)
			{
				maxout = outputs[j];
				indmax = j;
			}
		}
		swapf = outputs[i];
		outputs[i] = outputs[indmax];
		outputs[indmax]= swapf;

		swapi = Xs[i];
		Xs[i] = Xs[indmax];
		Xs[indmax]= swapi;

		swapi = Ys[i];
		Ys[i] = Ys[indmax];
		Ys[indmax]= swapi;

		swapi = heights[i];
		heights[i] =  heights[indmax];
		heights[indmax]= swapi;
	}


	pointsLeft=1;
	while(pointsLeft==1)
	{
		pointsLeft=0;

		for(i=0;i<size;i++)
		{
			float fCX;
			float fCY;
			float fCH;
			float fCW;

			if(data->vg_deleted[i]==1)
				continue;
				//a new cluster found -> init of the cluster params
			if(pointsLeft==0)
			{
				pointsLeft=1;

				cxNum=outputs[i]*((float)Xs[i]);
				cyNum=outputs[i]*((float)Ys[i]);
				chNum=outputs[i]*((float)heights[i]);
				Denom=outputs[i];
				totalOutput=outputs[i];
				data->vg_deleted[i]=1;
				votesInCluster=1;
				continue;
			}

				//check if the point falls inside the cluster
			fCX=cxNum/Denom;
			fCY=cyNum/Denom;
			fCH=chNum/Denom;
			fCW=SR*fCH;


				//here is the criterion
			if( Xs[i]<fCX+fCW/3.0 && Xs[i]>fCX-fCW/3.0 && Ys[i]<fCY+fCH/3.0 && Ys[i]>fCY-fCH/3.0 )
			{
				data->vg_deleted[i]=1;
				cxNum += outputs[i]*((float)Xs[i]);
				cyNum += outputs[i]*((float)Ys[i]);
				chNum += outputs[i]*((float)heights[i]);
				Denom += outputs[i];
				totalOutput += outputs[i];
				votesInCluster++;
			}
		}

		// now we have completed one cluster
		// and must put it on the image
		if(pointsLeft==0)
		{
			//delete[] deleted;
			return;
		}

		CX=(int)(cxNum/Denom);
		CY=(int)(cyNum/Denom);
		CH=(int)(chNum/Denom);
		CW=(int)(SR*(double)CH);


		//The final answer about the cluster should be the average
		Xf[*nb]=CX;
		Yf[*nb]=CY;
		Hf[*nb]=CH;

			// ...or the MAX answer (first seems to be better)
	//	Xf[*nb]=totalX;
	//	Yf[*nb]=totalY;
	//	Hf[*nb]=totalH;


		outf[*nb]=totalOutput;
		(*nb)++;

	}
}


int overlap(int i, int j, int Xri, int Yri, int Hri, int Wri, float outri,
		int Xrj, int Yrj, int Hrj, int Wrj, float outrj, float *perc)
{
  	int ind=-1;

  	int Xmini=Xri-Wri/2;
  	int Xmaxi=Xri+Wri/2;
  	int Ymini=Yri-Hri/2;
  	int Ymaxi=Yri+Hri/2;

  	int Xminj=Xrj-Wrj/2;
  	int Xmaxj=Xrj+Wrj/2;
  	int Yminj=Yrj-Hrj/2;
  	int Ymaxj=Yrj+Hrj/2;

  	float perci;
  	float percj;

  	int nb=0;
	int x, y;


/*
		//check based on the corners, quickly discards
		//non-overlapping windows
	int x=Xminj;
	int y=Yminj;

		//if the upper left corner does not lie inside, etc
	if( !((Xmini>=Xminj) && (Xmini<=Xmaxj) && (Ymini>=Yminj) && (Ymini<=Ymaxj))
		&& !((Xmini>=Xminj) && (Xmini<=Xmaxj) && (Ymaxi>=Yminj) && (Ymaxi<=Ymaxj))
		&& !((Xmaxi>=Xminj) && (Xmaxi<=Xmaxj) && (Ymini>=Yminj) && (Ymini<=Ymaxj))
		&& !((Xmaxi>=Xminj) && (Xmaxi<=Xmaxj) && (Ymaxi>=Yminj) && (Ymaxi<=Ymaxj))
		)
			return(-1);
*/

  	for(x=Xmini;x<=Xmaxi;x++)
      	for(y=Ymini;y<=Ymaxi;y++)
			if ((x>=Xminj) && (x<=Xmaxj) && (y>=Yminj) && (y<=Ymaxj))
				nb++;

  	perci = ((float)nb)/((float)(Hri*Wri));
  	percj = ((float)nb)/((float)(Hrj*Wrj));

  	if (perci>percj)
		*perc=perci;
	else
		*perc=percj;


	if ((*perc)>0.2)
	{
		if (outrj<outri)
			ind=j;
		else
			ind=i;
	}
	return ind;
}


int my_round(double x)
{
	double decimal=floor(x);
	if(x-decimal < 0.5)
		return( (int)x );
	else
		return( (int)(x+1.0) );
}



int L2_step = 0;


int searchFacesConvFineStatic(struct per_image_data *data, struct CNN *cnn, struct lpiImage *img_source, int S1, int S2, double ST, int *X, int *Y, int *H,
int *W, float *O)
{
	int iter;
	int Sm[64], itotal;
	int S;
	int Si;

	//check if we have a colored input or not
	struct lpiImage *img_input=img_source;

	//for each SCALE
	int nbS=0;
	int facesFound=0;

	int allClusters;
	int minVoteSize;

	int allSize;
	int n;

	int i, j;

	int nofFaces;

  /* precompute iterations */
	iter = 0;
	for(Si=S1; Si<=S2; Si=(int)floor((float)(Si)*ST)) {
		int width=img_source->width;
		int height=img_source->height;
		int h1 = (int)floor((double)height*((double)SH/(double)Si));
		int w1 = (int)floor((double)width*((double)SH/(double)Si));


		if(h1<36 || w1<32)
			continue;

		Sm[iter] = Si;
		iter++;
	}
	itotal = iter;

	printf("total iters : %d\n", itotal); fflush(0);


	//for(int S=S1; S<=S2; S=(int)floor((float)(S)*ST))
	for(iter = 0; iter< itotal; iter++)
	{
		int i, j;
		int width=img_source->width;
		int height=img_source->height;
		int nbS;

		{
		int h1;
		int w1;

		struct lpiImage *img_tmp;
		unsigned char *p1;
		float *p2;

		int current_width, current_height, total_size;

		float *pp;

		nbS = iter;
		S = Sm[iter];

		h1 = (int)floor((double)height*((double)SH/(double)S));
		w1 = (int)floor((double)width*((double)SH/(double)S));


		if(h1<36 || w1<32) {
			printf("this should not happen!\n");
			exit(1);
			continue;
		}

		data->imgt[nbS]=lpiImage_lpiCreateImage(w1, h1, sizeof(unsigned char));

		//CONVOLVE
		lpiImage_lpiResize(img_input, data->imgt[nbS]);

		img_tmp=lpiImage_lpiCreateImage(w1, h1, sizeof(float));


		p1=(unsigned char *)data->imgt[nbS]->imageData;
		p2=(float *)img_tmp->imageData;
		for(i=0;i<h1;i++)
			for(j=0;j<w1;j++)
				p2[i*w1+j]=(float)( ((int)p1[i*data->imgt[nbS]->width+j]) - 128)/((float)128.0);

		pp=CConvolver_ConvolveRoughlyStillImage(&data->cconv[nbS], img_tmp, w1, h1, &current_width, &current_height, &total_size, S);

		//CHECK THE RESULTS
		for(i=0;i<total_size;i++)
		{
			float output=pp[i];
			int y;
			int x;

			int ycenter;
			int xcenter;

			if (output<=(float)0.0)
				continue;

			y=i/current_width;
			x=i%current_width;

			ycenter = (int)floor((double)(4*y*S)/(double)SH) + (int)floor((double)S/2.0);
			xcenter = (int)floor((double)(4*x*S)/(double)SH) + (int)floor((double)S*SR/2.0);

			data->Xs[facesFound]=xcenter;
			data->Ys[facesFound]=ycenter;
			data->heights[facesFound]=S;
			data->outputs[facesFound]=output;
			facesFound++;

		}

			//FREE IMAGE
		lpiImage_lpiReleaseImage(&data->imgt[nbS]);
		CConvolver_DeallocateOutput(&data->cconv[nbS]);

		lpiImage_lpiReleaseImage(&img_tmp);

		}
	}


	/* Second phase: VoteGrouping2. */

	nbS = itotal+1;

	if(facesFound<1)
	{
		return(0);
	}
/////////////////////////////


	allClusters=0;
	minVoteSize=1;

	VoteGrouping2(data, data->Xs, data->Ys, data->outputs, data->heights, facesFound, minVoteSize, 0.0, data->Xf, data->Yf, data->outf, data->Hf, &allClusters);

///////////////////////////////////////////////////////////////////////////////


	/* Phase 3 ... */
	allSize=0;

	printf("allClusters = %d\n", allClusters);
	for(n=0;n<allClusters;n++)
	{
		//the various scales to try (Hf is the current target's height)
		float S11 = 0.8*data->Hf[n];
		float S21 = 1.5*data->Hf[n];

		//search through these scales with constant step 1/10 of space
		float dS = (S21-S11)/10.0;

		int recount=0;
		float maxOut=-1.0;
		int Xo=0;
		int Yo=0;
		float So=0.0;
		int XoS=data->Xf[n];
		int YoS=data->Yf[n];
		float mean=0.0;
		int k;
		float S;

		L2_step++;

		nbS=0;
		for(k=0, S=S11; k<10; k++, S+=dS)
		{
			if (k==0 || k==2 || k==4 || k==5)
			{
				recount += CropRescaleConvolve(cnn, img_input, S, XoS, YoS, &maxOut, &Xo, &Yo, &So, &mean);
			}
		}
			//throw away a lot of targets with quite low volume
		if(mean<10.0 || recount==0)
			continue;

		data->allX[allSize]=my_round((double)Xo);
		data->allY[allSize]=my_round((double)Yo);
		data->allH[allSize]=my_round((double)So);
		data->allOut[allSize]=mean;
		allSize++;

	}
////////////////////////////////////////////////////////////////////////////////////////////

	/* Phase 4  */
	for(i=0;i<allSize;i++)
		data->discarded[i]=0;

	for(i=0;i<allSize;i++)
	{
		for(j=i+1;j<allSize;j++)
		{
			float perc=0.0;
			int r;

			if(data->discarded[i]==1 || data->discarded[j]==1)
				continue;
			r=overlap(i, j, data->allX[i], data->allY[i], data->allH[i], (int)(SR*data->allH[i]), data->allOut[i],
				data->allX[j], data->allY[j], data->allH[j], (int)(SR*data->allH[j]), data->allOut[j], &perc);
			if(r==i)
				data->discarded[i]=1;
			else if(r==j)
				data->discarded[j]=1;
		}
	}


	/* Output */
	nofFaces=0;
	for(i=0;i<allSize;i++)
	{
		int Sw;
		if(data->discarded[i]==1)
			continue;
		Sw = (int)(SR*data->allH[i]);

		drawFace(img_source, data->allX[i], data->allY[i], Sw, data->allH[i], data->allOut[i], 1, img_input, i);
		X[nofFaces]=data->allX[i];
		Y[nofFaces]=data->allY[i];
		H[nofFaces]=data->allH[i];
		W[nofFaces]=Sw;
		O[nofFaces]=data->allOut[i];
		nofFaces++;
	}

	return(nofFaces);
}







