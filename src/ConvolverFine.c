
#include "ConvolverFine.h"
#include <math.h>
#include "Convolver.h"


	//for the lookup implementation of the TANH
#include "tanh_lup.h"
extern float TANH_LUP[1600];


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


void CConvolverFine_SetCNN(struct CConvolverFine *CConvolverFine_v, struct CNN *cnn)
{
	CConvolverFine_v->m_cnn=cnn;
}


float *CConvolverFine_ConvolveFine(struct CConvolverFine *CConvolverFine_v, struct lpiImage *img, float *res)
{
	int i;

	struct lpiImage **fm0;

	int current_width;
	int current_height;
	//NOTE: the four different kind of displacements
	struct lpiImage **fm0SubD1;
	struct lpiImage **fm0SubD2;
	struct lpiImage **fm0SubD3;
	struct lpiImage **fm0SubD4;

	CConvolverFine_v->m_width=img->width;
	CConvolverFine_v->m_height=img->height;

	/////////////////1st layer//////////////////////

	//convolution
	fm0=(struct lpiImage **)calloc(4, sizeof(struct lpiImage *));

	//subsampling
	current_width=(CConvolverFine_v->m_width-4)/2;
	current_height=(CConvolverFine_v->m_height-4)/2;
	//NOTE: the four different kind of displacements
	fm0SubD1=(struct lpiImage **)calloc(4, sizeof(struct lpiImage *));
	fm0SubD2=(struct lpiImage **)calloc(4, sizeof(struct lpiImage *));
	fm0SubD3=(struct lpiImage **)calloc(4, sizeof(struct lpiImage *));
	fm0SubD4=(struct lpiImage **)calloc(4, sizeof(struct lpiImage *));

	for(i=0;i<4;i++)
	{
		fm0[i]=CConvolverFine_CreateImage(CConvolverFine_v->m_width-4, CConvolverFine_v->m_height-4);
		CConvolver_Convolve(img, CConvolverFine_v->m_cnn->m_kernels0[i].kern, CConvolverFine_v->m_cnn->m_kernels0[i].bias, 5, fm0[i]);

		fm0SubD1[i]=CConvolverFine_CreateImage(current_width, current_height);
		fm0SubD2[i]=CConvolverFine_CreateImage(current_width, current_height);
		fm0SubD3[i]=CConvolverFine_CreateImage(current_width, current_height);
		fm0SubD4[i]=CConvolverFine_CreateImage(current_width, current_height);

		CConvolverFine_SubSample(fm0[i], CConvolverFine_v->m_cnn->m_kernels0[i].coeff, CConvolverFine_v->m_cnn->m_kernels0[i].sbias, 0, 0, 0, fm0SubD1[i]);
		CConvolverFine_SubSample(fm0[i], CConvolverFine_v->m_cnn->m_kernels0[i].coeff, CConvolverFine_v->m_cnn->m_kernels0[i].sbias, 0, 0, 1, fm0SubD2[i]);
		CConvolverFine_SubSample(fm0[i], CConvolverFine_v->m_cnn->m_kernels0[i].coeff, CConvolverFine_v->m_cnn->m_kernels0[i].sbias, 0, 1, 0, fm0SubD3[i]);
		CConvolverFine_SubSample(fm0[i], CConvolverFine_v->m_cnn->m_kernels0[i].coeff, CConvolverFine_v->m_cnn->m_kernels0[i].sbias, 0, 1, 1, fm0SubD4[i]);
	}



	CConvolverFine_SecondLayer(CConvolverFine_v, fm0SubD1, 0, 0, res);
	CConvolverFine_SecondLayer(CConvolverFine_v, fm0SubD2, 0, 1, res);
	CConvolverFine_SecondLayer(CConvolverFine_v, fm0SubD3, 1, 0, res);
	CConvolverFine_SecondLayer(CConvolverFine_v, fm0SubD4, 1, 1, res);



	//FREE memory used in this function
	for(i=0;i<4;i++)
	{
		lpiImage_lpiReleaseImage(&fm0[i]);
		lpiImage_lpiReleaseImage(&fm0SubD1[i]);
		lpiImage_lpiReleaseImage(&fm0SubD2[i]);
		lpiImage_lpiReleaseImage(&fm0SubD3[i]);
		lpiImage_lpiReleaseImage(&fm0SubD4[i]);
	}
	free(fm0);
	free(fm0SubD1);
	free(fm0SubD2);
	free(fm0SubD3);
	free(fm0SubD4);

	return(NULL);
}



void CConvolverFine_SecondLayer(struct CConvolverFine *CConvolverFine_v, struct lpiImage **fm0, int dispx, int dispy, float *res)
{
	int width=fm0[0]->width;
	int height=fm0[0]->height;
	int i;

	struct lpiImage **fm1;

	int current_width;
	int current_height;
	struct lpiImage **fm1SubD1;
	struct lpiImage **fm1SubD2;
	struct lpiImage **fm1SubD3;
	struct lpiImage **fm1SubD4;

	//FMs addition
	struct lpiImage **fm1In=(struct lpiImage **)calloc(20, sizeof(struct lpiImage *));

	for(i=0;i<8;i++)
		fm1In[i]=fm0[i/2];
	fm1In[8]=fm0[0];
	fm1In[9]=fm0[0];
	fm1In[10]=fm0[0];
	fm1In[11]=fm0[1];
	fm1In[12]=fm0[1];
	fm1In[13]=fm0[2];

	fm1In[14]=fm0[1];
	fm1In[15]=fm0[2];
	fm1In[16]=fm0[3];
	fm1In[17]=fm0[2];
	fm1In[18]=fm0[3];
	fm1In[19]=fm0[3];

	fm1=(struct lpiImage **)calloc(14, sizeof(struct lpiImage *));
	for(i=0;i<14;i++)
		fm1[i]=CConvolverFine_CreateImage(width-2, height-2);

	//convolution
	for(i=0;i<8;i++)
	{
		CConvolver_Convolve(fm1In[i], CConvolverFine_v->m_cnn->m_kernels1[i].kern, CConvolverFine_v->m_cnn->m_kernels1[i].bias, 3, fm1[i]);
	}
	for(i=8;i<14;i++)
	{
		CConvolver_Convolve2(fm1In[i], fm1In[i+6], CConvolverFine_v->m_cnn->m_kernels1[i].kern, CConvolverFine_v->m_cnn->m_kernels1[i].kern2,
			CConvolverFine_v->m_cnn->m_kernels1[i].bias, 3, fm1[i]);
	}


	//sub-sampling
	current_width=(width-2)/2;
	current_height=(height-2)/2;
	fm1SubD1=(struct lpiImage **)calloc(14, sizeof(struct lpiImage *));
	fm1SubD2=(struct lpiImage **)calloc(14, sizeof(struct lpiImage *));
	fm1SubD3=(struct lpiImage **)calloc(14, sizeof(struct lpiImage *));
	fm1SubD4=(struct lpiImage **)calloc(14, sizeof(struct lpiImage *));
	for(i=0;i<14;i++)
	{
		fm1SubD1[i]=CConvolverFine_CreateImage(current_width, current_height);
		fm1SubD2[i]=CConvolverFine_CreateImage(current_width, current_height);
		fm1SubD3[i]=CConvolverFine_CreateImage(current_width, current_height);
		fm1SubD4[i]=CConvolverFine_CreateImage(current_width, current_height);
	}


	for(i=0;i<14;i++)
	{
		CConvolverFine_SubSample(fm1[i], CConvolverFine_v->m_cnn->m_kernels1[i].coeff, CConvolverFine_v->m_cnn->m_kernels1[i].sbias, 0, 0, 0, fm1SubD1[i]);
		CConvolverFine_SubSample(fm1[i], CConvolverFine_v->m_cnn->m_kernels1[i].coeff, CConvolverFine_v->m_cnn->m_kernels1[i].sbias, 0, 0, 1, fm1SubD2[i]);
		CConvolverFine_SubSample(fm1[i], CConvolverFine_v->m_cnn->m_kernels1[i].coeff, CConvolverFine_v->m_cnn->m_kernels1[i].sbias, 0, 1, 0, fm1SubD3[i]);
		CConvolverFine_SubSample(fm1[i], CConvolverFine_v->m_cnn->m_kernels1[i].coeff, CConvolverFine_v->m_cnn->m_kernels1[i].sbias, 0, 1, 1, fm1SubD4[i]);
	}


	CConvolverFine_NeuronLayer(CConvolverFine_v, fm1SubD1, dispx, dispy, 0, 0, res);
	CConvolverFine_NeuronLayer(CConvolverFine_v, fm1SubD2, dispx, dispy, 1, 0, res);
	CConvolverFine_NeuronLayer(CConvolverFine_v, fm1SubD3, dispx, dispy, 0, 1, res);
	CConvolverFine_NeuronLayer(CConvolverFine_v, fm1SubD4, dispx, dispy, 1, 1, res);



	for(i=0;i<14;i++)
	{
		lpiImage_lpiReleaseImage(&fm1[i]);
		lpiImage_lpiReleaseImage(&fm1SubD1[i]);
		lpiImage_lpiReleaseImage(&fm1SubD2[i]);
		lpiImage_lpiReleaseImage(&fm1SubD3[i]);
		lpiImage_lpiReleaseImage(&fm1SubD4[i]);
	}
	free(fm1In);
	free(fm1);
	free(fm1SubD1);
	free(fm1SubD2);
	free(fm1SubD3);
	free(fm1SubD4);
}


void CConvolverFine_NeuronLayer(struct CConvolverFine *CConvolverFine_v, struct lpiImage **fm, int dispX1, int dispY1, int dispX2, int dispY2, float *res)
{
	int width=fm[0]->width;
	int height=fm[0]->height;
	int i, j;

	int current_width = width - 6 + 1;
	int current_height = height - 7 + 1;

	struct lpiImage **fm2=(struct lpiImage **)calloc(14, sizeof(struct lpiImage *));

	struct lpiImage *fmOut;
	float *pOut;

	int CC_m_width = CConvolverFine_v->m_width;

	for(i=0;i<14;i++)
		fm2[i]=CConvolverFine_CreateImage(current_width, current_height);

	for(i=0;i<14;i++)
		CConvolverFine_ConvolveNeuron(CConvolverFine_v, fm[i], i, fm2[i]);

	//output layer
	fmOut=CConvolverFine_CreateImage(current_width, current_height);


	CConvolver_ConvolveOutput(fm2, 14, CConvolverFine_v->m_cnn->m_kernels3[0].kern, CConvolverFine_v->m_cnn->m_kernels3[0].bias,
		current_width, current_height, fmOut);


	pOut=(float *)fmOut->imageData;
	//now put the final results into the array

	for(i=0;i<current_height;i++) {
		int i4_etc = (4*i + 2*dispY2 + dispY1)*CC_m_width + 2*dispX2 + dispX1;
		int i_curr_width = i*current_width;
		for(j=0;j<current_width;j++)
			res[ i4_etc + 4*j] = pOut[i_curr_width+j];
	}


	for(i=0;i<14;i++)
		lpiImage_lpiReleaseImage(&fm2[i]);
	free(fm2);

	lpiImage_lpiReleaseImage(&fmOut);
}

struct lpiImage *CConvolverFine_CreateImage(int width, int height)
{
	struct lpiImage *img = lpiImage_lpiCreateImage(width, height, sizeof(float));
	return(img);
}

void CConvolverFine_SubSample(struct lpiImage *fm, float coeff, float bias, int disp, int x_disp, int y_disp, struct lpiImage *sub)
{
	//subsampling
	float *pb=(float *)fm->imageData;
	float *pc=(float *)sub->imageData;
	int ih2=sub->height;
	int iw2=sub->width;
	int i, j, w=fm->width;

	for(i=0;i<ih2;i++) {
		int i2=2*i+disp;
		int i2y_disp = i2 + y_disp;
		int i2y_disp_w = i2y_disp*w;
		float *pb1 = &pb[i2y_disp_w+x_disp];
		int i_iw2 = i*iw2;
		float *pc1 = &pc[i_iw2];

		for(j=0;j<iw2;j++)
		{
			int j2=2*j+disp;
			register float sum= (pb1[j2] + pb1[j2+1] + pb1[w + j2] + pb1[w+j2+1])*coeff + bias;
	//		sum = c_la * tanh( (double)(c_lb*sum) );
			TANH_SIMPLE(sum, sum);

			pc1[j]=sum;
		}
	}

}

//#define MULTIASSIGN

void CConvolverFine_ConvolveNeuron(struct CConvolverFine *CConvolverFine_v, struct lpiImage *fm, int index, struct lpiImage *out)
{
//	int outWidth = fm->width - kernel_width + 1;
//	int outHeight = fm->height - kernel_height + 1;
//	kernel_width = 6
//	kernel_heigth = 7

//	int wsWidth=kernel_width/2;
//	int wsHeight=kernel_height/2;

//	int wsWidth=3;
//	int wsHeight=3;

	float *pfm=(float *)fm->imageData;
	float *pout=(float *)out->imageData;

	int wfm=fm->width;
	int wout=out->width;

	float bias=CConvolverFine_v->m_cnn->m_kernels2[index].bias;
	float *kernel=CConvolverFine_v->m_cnn->m_kernels2[index].kern;
//	int wkern=6;

	int i, j;
//	int l, k;

	for(i=3;i<fm->height-3;i++)
		for(j=3;j<=fm->width-3;j++)
		{
			float product=bias;
			float tmp;
			int i_wfm_j = i*wfm+j;
			int i_k_wfm_j;
			//float input;
			float *kernel1;
			float *pfm1;

			//for every weight of the conv. kernel
//			k=-3;
			{
				i_k_wfm_j = i_wfm_j + (-3)*wfm;
				kernel1 = &kernel[3];
				pfm1 = &pfm[i_k_wfm_j];
#ifdef MULTIASSIGN
				product += kernel1[-3] * pfm1[-3];
				product += kernel1[-2] * pfm1[-2];
				product += kernel1[-1] * pfm1[-1];
				product += kernel1[ 0] * pfm1[ 0];
				product += kernel1[ 1] * pfm1[ 1];
				product += kernel1[ 2] * pfm1[ 2];
#else
				product += kernel1[-3] * pfm1[-3] + kernel1[-2] * pfm1[-2] + kernel1[-1] * pfm1[-1]
						+ kernel1[ 0] * pfm1[ 0] + kernel1[ 1] * pfm1[ 1] + kernel1[ 2] * pfm1[ 2];
#endif
			}
//			k=-2;
			{
				i_k_wfm_j = i_wfm_j + (-2)*wfm;
				kernel1 = &kernel[9];
				pfm1 = &pfm[i_k_wfm_j];
#ifdef MULTIASSIGN
				product += kernel1[-3] * pfm1[-3];
				product += kernel1[-2] * pfm1[-2];
				product += kernel1[-1] * pfm1[-1];
				product += kernel1[ 0] * pfm1[ 0];
				product += kernel1[ 1] * pfm1[ 1];
				product += kernel1[ 2] * pfm1[ 2];
#else
				product += kernel1[-3] * pfm1[-3] + kernel1[-2] * pfm1[-2] + kernel1[-1] * pfm1[-1]
						+ kernel1[ 0] * pfm1[ 0] + kernel1[ 1] * pfm1[ 1] + kernel1[ 2] * pfm1[ 2];
#endif
			}
//			k=-1;
			{
				i_k_wfm_j = i_wfm_j + (-1)*wfm;
				kernel1 = &kernel[15];
				pfm1 = &pfm[i_k_wfm_j];
#ifdef MULTIASSIGN
				product += kernel1[-3] * pfm1[-3];
				product += kernel1[-2] * pfm1[-2];
				product += kernel1[-1] * pfm1[-1];
				product += kernel1[ 0] * pfm1[ 0];
				product += kernel1[ 1] * pfm1[ 1];
				product += kernel1[ 2] * pfm1[ 2];
#else
				product += kernel1[-3] * pfm1[-3] + kernel1[-2] * pfm1[-2] + kernel1[-1] * pfm1[-1]
						+ kernel1[ 0] * pfm1[ 0] + kernel1[ 1] * pfm1[ 1] + kernel1[ 2] * pfm1[ 2];
#endif
			}
//			k=0;
			{
				i_k_wfm_j = i_wfm_j + 0*wfm;
				pfm1 = &pfm[i_k_wfm_j];
				kernel1 = &kernel[21];
#ifdef MULTIASSIGN
				product += kernel1[-3] * pfm1[-3];
				product += kernel1[-2] * pfm1[-2];
				product += kernel1[-1] * pfm1[-1];
				product += kernel1[ 0] * pfm1[ 0];
				product += kernel1[ 1] * pfm1[ 1];
				product += kernel1[ 2] * pfm1[ 2];
#else
				product += kernel1[-3] * pfm1[-3] + kernel1[-2] * pfm1[-2] + kernel1[-1] * pfm1[-1]
						+ kernel1[ 0] * pfm1[ 0] + kernel1[ 1] * pfm1[ 1] + kernel1[ 2] * pfm1[ 2];
#endif
			}
//			k=1;
			{
				i_k_wfm_j = i_wfm_j + 1*wfm;
				kernel1 = &kernel[27];
				pfm1 = &pfm[i_k_wfm_j];
#ifdef MULTIASSIGN
				product += kernel1[-3] * pfm1[-3];
				product += kernel1[-2] * pfm1[-2];
				product += kernel1[-1] * pfm1[-1];
				product += kernel1[ 0] * pfm1[ 0];
				product += kernel1[ 1] * pfm1[ 1];
				product += kernel1[ 2] * pfm1[ 2];
#else
				product += kernel1[-3] * pfm1[-3] + kernel1[-2] * pfm1[-2] + kernel1[-1] * pfm1[-1]
						+ kernel1[ 0] * pfm1[ 0] + kernel1[ 1] * pfm1[ 1] + kernel1[ 2] * pfm1[ 2];
#endif
			}
//			k=2;
			{
				i_k_wfm_j = i_wfm_j + 2*wfm;
				kernel1 = &kernel[33];
				pfm1 = &pfm[i_k_wfm_j];
#ifdef MULTIASSIGN
				product += kernel1[-3] * pfm1[-3];
				product += kernel1[-2] * pfm1[-2];
				product += kernel1[-1] * pfm1[-1];
				product += kernel1[ 0] * pfm1[ 0];
				product += kernel1[ 1] * pfm1[ 1];
				product += kernel1[ 2] * pfm1[ 2];
#else
				product += kernel1[-3] * pfm1[-3] + kernel1[-2] * pfm1[-2] + kernel1[-1] * pfm1[-1]
						+ kernel1[ 0] * pfm1[ 0] + kernel1[ 1] * pfm1[ 1] + kernel1[ 2] * pfm1[ 2];
#endif
			}
//			k=3;
			{
				i_k_wfm_j = i_wfm_j + 3*wfm;
				kernel1 = &kernel[39];
				pfm1 = &pfm[i_k_wfm_j];
#ifdef MULTIASSIGN
				product += kernel1[-3] * pfm1[-3];
				product += kernel1[-2] * pfm1[-2];
				product += kernel1[-1] * pfm1[-1];
				product += kernel1[ 0] * pfm1[ 0];
				product += kernel1[ 1] * pfm1[ 1];
				product += kernel1[ 2] * pfm1[ 2];
#else
				product += kernel1[-3] * pfm1[-3] + kernel1[-2] * pfm1[-2] + kernel1[-1] * pfm1[-1]
						+ kernel1[ 0] * pfm1[ 0] + kernel1[ 1] * pfm1[ 1] + kernel1[ 2] * pfm1[ 2];
#endif
			}


		//	float tmp = c_la * tanh( (double)(c_lb*product) );
			TANH_SIMPLE(product, tmp);

			
		//	pout[ (i-wsHeight)*wout + j-wsWidth ] = tmp;
			pout[ (i-3)*wout + j-3 ] = tmp;
		}
}

