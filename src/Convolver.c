// Convolver.cpp: implementation of the CConvolver class.
//
//////////////////////////////////////////////////////////////////////

#include "Convolver.h"
#include <math.h>
extern double my_gettime();

//for the lookup implementation of the TANH
#include "tanh_lup.h"
extern float TANH_LUP[1600];

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

void CConvolver_SetCNN(struct CConvolver *CConvolver_v, struct CNN *cnn)
{
	CConvolver_v->m_cnn=cnn;
}

float *CConvolver_ConvolveRoughly(struct CConvolver *CConvolver_v, struct lpiImage *img, int *ww, int *hh, int *tt)
{
	int i;
	int current_width;
	int current_height;
	float *pOut;
	int total_size;

	/////////////////1st layer//////////////////////

	//convolution + subsampling
	for(i=0;i<4;i++)
	{
		CConvolver_Convolve(img, CConvolver_v->m_cnn->m_kernels0[i].kern, CConvolver_v->m_cnn->m_kernels0[i].bias, 5, CConvolver_v->fm0[i]);

		CConvolver_SubSample(CConvolver_v->fm0[i], CConvolver_v->m_cnn->m_kernels0[i].coeff, CConvolver_v->m_cnn->m_kernels0[i].sbias, 0, CConvolver_v->fm0Sub[i]);

	}

	current_width=(CConvolver_v->width-4)/2;
	current_height=(CConvolver_v->height-4)/2;


	///////////// PREPARE 2nd LAYER /////////////////////////
	for(i=0;i<8;i++)
		CConvolver_v->fm1In[i]=CConvolver_v->fm0Sub[i/2];


	CConvolver_v->fm1In[8]=CConvolver_v->fm0Sub[0];
	CConvolver_v->fm1In[9]=CConvolver_v->fm0Sub[0];
	CConvolver_v->fm1In[10]=CConvolver_v->fm0Sub[0];
	CConvolver_v->fm1In[11]=CConvolver_v->fm0Sub[1];
	CConvolver_v->fm1In[12]=CConvolver_v->fm0Sub[1];
	CConvolver_v->fm1In[13]=CConvolver_v->fm0Sub[2];

	CConvolver_v->fm1In[14]=CConvolver_v->fm0Sub[1];
	CConvolver_v->fm1In[15]=CConvolver_v->fm0Sub[2];
	CConvolver_v->fm1In[16]=CConvolver_v->fm0Sub[3];
	CConvolver_v->fm1In[17]=CConvolver_v->fm0Sub[2];
	CConvolver_v->fm1In[18]=CConvolver_v->fm0Sub[3];
	CConvolver_v->fm1In[19]=CConvolver_v->fm0Sub[3];

	////////////////////2nd layer//////////////////////

	//convolution + subsampling
	for(i=0;i<8;i++)
	{
		CConvolver_Convolve(CConvolver_v->fm1In[i], CConvolver_v->m_cnn->m_kernels1[i].kern, CConvolver_v->m_cnn->m_kernels1[i].bias, 3, CConvolver_v->fm1[i]);

		CConvolver_SubSample(CConvolver_v->fm1[i], CConvolver_v->m_cnn->m_kernels1[i].coeff, CConvolver_v->m_cnn->m_kernels1[i].sbias, 0, CConvolver_v->fm1Sub[i]);

	}
	for(i=8;i<14;i++)
	{
		CConvolver_Convolve2(CConvolver_v->fm1In[i], CConvolver_v->fm1In[i+6], CConvolver_v->m_cnn->m_kernels1[i].kern, CConvolver_v->m_cnn->m_kernels1[i].kern2,
			CConvolver_v->m_cnn->m_kernels1[i].bias, 3, CConvolver_v->fm1[i]);
		CConvolver_SubSample(CConvolver_v->fm1[i], CConvolver_v->m_cnn->m_kernels1[i].coeff, CConvolver_v->m_cnn->m_kernels1[i].sbias, 0, CConvolver_v->fm1Sub[i]);
	}

	current_width=(current_width-2)/2;
	current_height=(current_height-2)/2;


	///////////////////3rd layer //////////////////////////////////////////////

	current_width = current_width - 6 + 1;
	current_height = current_height - 7 + 1;


	for(i=0;i<14;i++)
		CConvolver_ConvolveNeuron(CConvolver_v, CConvolver_v->fm1Sub[i], i, CConvolver_v->fm2[i]);

	/////////////////  OUTPUT LAYER !!!   //////////////////

	CConvolver_ConvolveOutput(CConvolver_v->fm2, 14, CConvolver_v->m_cnn->m_kernels3[0].kern, CConvolver_v->m_cnn->m_kernels3[0].bias,
		current_width, current_height, CConvolver_v->fmOut);


	pOut=(float *)CConvolver_v->fmOut->imageData;
	total_size=current_width*current_height;


	*hh=current_height;
	*ww=current_width;
	*tt=total_size;

	return(pOut);
}


struct lpiImage *CConvolver_CreateImage(int width, int height)
{
	struct lpiImage *img=lpiImage_lpiCreateImage(width, height, sizeof(float));
	return(img);
}

void CConvolver_SubSample(struct lpiImage *fm, float coeff, float bias, int disp, struct lpiImage *sub)
{
	//subsampling
	float *pb=(float *)fm->imageData;
	float *pc=(float *)sub->imageData;
	int ih2=sub->height;
	int iw2=sub->width;
	int i, j, w=fm->width;
	int sub_size;

	for(i=0;i<ih2;i++)
	{
		int i2=2*i+disp;
		int i2w = i2*w;
		int i_iw2 = i*iw2;
		float *pc1 = &pc[i_iw2];
		for(j=0;j<iw2;j++)
		{
			int j2=2*j+disp;
			float *pb1 = &pb[i2w+j2];

			pc1[j]= (pb1[0] + pb1[1] + pb1[w] + pb1[w+1])*coeff + bias;

		}
	}

	sub_size=ih2*iw2;
	for(i=0;i<sub_size;i++)
	{
		TANH_SIMPLE(pc[i], pc[i]);
	}
}

//#define MULTIASSIGN

void CConvolver_ConvolveNeuron(struct CConvolver *CConvolver_v, struct lpiImage *fm, int index, struct lpiImage *out)
{
	float *pfm=(float *)fm->imageData;
	float *pout=(float *)out->imageData;

	int wfm=fm->width;
	int wout=out->width;

	float bias=CConvolver_v->m_cnn->m_kernels2[index].bias;
	float *kernel=CConvolver_v->m_cnn->m_kernels2[index].kern;

	int i, j;
//	int l, k;

	for(i=3;i<fm->height-3;i++) {
		int i_3_wout_3 = (i-3)*wout-3;
		float *pout1 = &pout[i_3_wout_3];
		for(j=3;j<=fm->width-3;j++)
		{
			float product=bias;
			float tmp;
			int i_wfm_j = i*wfm+j;
			int i_k_wfm_j;
			float *pfm1;
			float *kernel1;

			//for every weight of the conv. kernel
//			k=-3;
			{
				i_k_wfm_j = i_wfm_j -3*wfm;
				pfm1 = &pfm[i_k_wfm_j];
				kernel1 = &kernel[3];
#ifdef MULTIASSIGN
				product += kernel1[-3] * pfm1[-3];
				product += kernel1[-2] * pfm1[-2];
				product += kernel1[-1] * pfm1[-1];
				product += kernel1[0] * pfm1[0];
				product += kernel1[1] * pfm1[1];
				product += kernel1[2] * pfm1[2];
#else
				product += kernel1[-3] * pfm1[-3] + kernel1[-2] * pfm1[-2] 
						+ kernel1[-1] * pfm1[-1] + kernel1[0] * pfm1[0] 
						+ kernel1[1] * pfm1[1] + kernel1[2] * pfm1[2];
#endif
			}
//			k=-2;
			{
				i_k_wfm_j = i_wfm_j -2*wfm;
				pfm1 = &pfm[i_k_wfm_j];
				kernel1 = &kernel[9];
#ifdef MULTIASSIGN
				product += kernel1[-3] * pfm1[-3];
				product += kernel1[-2] * pfm1[-2];
				product += kernel1[-1] * pfm1[-1];
				product += kernel1[0] * pfm1[0];
				product += kernel1[1] * pfm1[1];
				product += kernel1[2] * pfm1[2];
#else
				product += kernel1[-3] * pfm1[-3] + kernel1[-2] * pfm1[-2] 
						+ kernel1[-1] * pfm1[-1] + kernel1[0] * pfm1[0] 
						+ kernel1[1] * pfm1[1] + kernel1[2] * pfm1[2];
#endif

			}
//			k=-1;
			{
				i_k_wfm_j = i_wfm_j - wfm;
				pfm1 = &pfm[i_k_wfm_j];
				kernel1 = &kernel[15];
#ifdef MULTIASSIGN
				product += kernel1[-3] * pfm1[-3];
				product += kernel1[-2] * pfm1[-2];
				product += kernel1[-1] * pfm1[-1];
				product += kernel1[0] * pfm1[0];
				product += kernel1[1] * pfm1[1];
				product += kernel1[2] * pfm1[2];
#else
				product += kernel1[-3] * pfm1[-3] + kernel1[-2] * pfm1[-2] 
						+ kernel1[-1] * pfm1[-1] + kernel1[0] * pfm1[0] 
						+ kernel1[1] * pfm1[1] + kernel1[2] * pfm1[2];
#endif
			}
//			k=0;
			{
				i_k_wfm_j = i_wfm_j;
				pfm1 = &pfm[i_k_wfm_j];
				kernel1 = &kernel[21];
#ifdef MULTIASSIGN
				product += kernel1[-3] * pfm1[-3];
				product += kernel1[-2] * pfm1[-2];
				product += kernel1[-1] * pfm1[-1];
				product += kernel1[0] * pfm1[0];
				product += kernel1[1] * pfm1[1];
				product += kernel1[2] * pfm1[2];
#else
				product += kernel1[-3] * pfm1[-3] + kernel1[-2] * pfm1[-2] 
						+ kernel1[-1] * pfm1[-1] + kernel1[0] * pfm1[0] 
						+ kernel1[1] * pfm1[1] + kernel1[2] * pfm1[2];
#endif
			}
//			k=1;
			{
				i_k_wfm_j = i_wfm_j + 1*wfm;
				pfm1 = &pfm[i_k_wfm_j];
				kernel1 = &kernel[27];
#ifdef MULTIASSIGN
				product += kernel1[-3] * pfm1[-3];
				product += kernel1[-2] * pfm1[-2];
				product += kernel1[-1] * pfm1[-1];
				product += kernel1[0] * pfm1[0];
				product += kernel1[1] * pfm1[1];
				product += kernel1[2] * pfm1[2];
#else
				product += kernel1[-3] * pfm1[-3] + kernel1[-2] * pfm1[-2] 
						+ kernel1[-1] * pfm1[-1] + kernel1[0] * pfm1[0] 
						+ kernel1[1] * pfm1[1] + kernel1[2] * pfm1[2];
#endif
			}
//			k=2;
			{
				i_k_wfm_j = i_wfm_j + 2*wfm;
				pfm1 = &pfm[i_k_wfm_j];
				kernel1 = &kernel[33];
#ifdef MULTIASSIGN
				product += kernel1[-3] * pfm1[-3];
				product += kernel1[-2] * pfm1[-2];
				product += kernel1[-1] * pfm1[-1];
				product += kernel1[0] * pfm1[0];
				product += kernel1[1] * pfm1[1];
				product += kernel1[2] * pfm1[2];
#else
				product += kernel1[-3] * pfm1[-3] + kernel1[-2] * pfm1[-2] 
						+ kernel1[-1] * pfm1[-1] + kernel1[0] * pfm1[0] 
						+ kernel1[1] * pfm1[1] + kernel1[2] * pfm1[2];
#endif
			}
//			k=3;
			{
				i_k_wfm_j = i_wfm_j + 3*wfm;
				pfm1 = &pfm[i_k_wfm_j];
				kernel1 = &kernel[39];
#ifdef MULTIASSIGN
				product += kernel1[-3] * pfm1[-3];
				product += kernel1[-2] * pfm1[-2];
				product += kernel1[-1] * pfm1[-1];
				product += kernel1[0] * pfm1[0];
				product += kernel1[1] * pfm1[1];
				product += kernel1[2] * pfm1[2];
#else
				product += kernel1[-3] * pfm1[-3] + kernel1[-2] * pfm1[-2] 
						+ kernel1[-1] * pfm1[-1] + kernel1[0] * pfm1[0] 
						+ kernel1[1] * pfm1[1] + kernel1[2] * pfm1[2];
#endif
			}
			TANH_SIMPLE(product, tmp);
			pout1[j] = tmp;
		}
	}
}



//just follow the path of ConvolveRoughly but just do the memory allocations
int CConvolver_InitFMs(struct CConvolver *CConvolver_v, int width, int height)
{
	int i;
	int current_width;
	int current_height;

	CConvolver_v->width=width;
	CConvolver_v->height=height;
	CConvolver_v->fm0=(struct lpiImage **)calloc(4, sizeof(struct lpiImage *));
	for(i=0;i<4;i++)
		CConvolver_v->fm0[i]=CConvolver_CreateImage(width, height);


	//subsampling
	current_width=(width-4)/2;
	current_height=(height-4)/2;
	CConvolver_v->fm0Sub=(struct lpiImage **)calloc(4, sizeof(struct lpiImage *));
	for(i=0;i<4;i++)
		CConvolver_v->fm0Sub[i]=CConvolver_CreateImage(current_width, current_height);


	///////////// PREPARE 2nd LAYER /////////////////////////
	CConvolver_v->fm1In=(struct lpiImage **)calloc(20, sizeof(struct lpiImage *));

	CConvolver_v->fm1=(struct lpiImage **)calloc(14, sizeof(struct lpiImage *));
	for(i=0;i<14;i++)
		CConvolver_v->fm1[i]=CConvolver_CreateImage(current_width, current_height);
	CConvolver_v->fm_fus1=CConvolver_CreateImage(current_width, current_height);
	CConvolver_v->fm_fus2=CConvolver_CreateImage(current_width, current_height);


	////////////////////2nd layer//////////////////////
	current_width=(current_width-2)/2;
	current_height=(current_height-2)/2;
	CConvolver_v->fm1Sub=(struct lpiImage **)calloc(14, sizeof(struct lpiImage *));
	for(i=0;i<14;i++)
		CConvolver_v->fm1Sub[i]=CConvolver_CreateImage(current_width, current_height);



	///////////////////3rd layer //////////////////////////////////////////////
	current_width = current_width - 6 + 1;
	current_height = current_height - 7 + 1;
	CConvolver_v->fm2=(struct lpiImage **)calloc(14, sizeof(struct lpiImage *));

	for(i=0;i<14;i++)
		CConvolver_v->fm2[i]=CConvolver_CreateImage(current_width, current_height);


	/////////////////  OUTPUT LAYER !!!   //////////////////
	CConvolver_v->fmOut=CConvolver_CreateImage(current_width, current_height);

	return(1);
}



void CConvolver_FreeFMs(struct CConvolver *CConvolver_v)
{
	int i;
	// FREE MEMORY !!!
	for(i=0;i<4;i++)
	{
		lpiImage_lpiReleaseImage(&CConvolver_v->fm0[i]);
		lpiImage_lpiReleaseImage(&CConvolver_v->fm0Sub[i]);
	}
	free(CConvolver_v->fm0);
	free(CConvolver_v->fm0Sub);
	free(CConvolver_v->fm1In);

	for(i=0;i<14;i++)
	{
		lpiImage_lpiReleaseImage(&CConvolver_v->fm1[i]);
		lpiImage_lpiReleaseImage(&CConvolver_v->fm1Sub[i]);
		lpiImage_lpiReleaseImage(&CConvolver_v->fm2[i]);
	}
	free(CConvolver_v->fm1);
	free(CConvolver_v->fm1Sub);
	free(CConvolver_v->fm2);

	lpiImage_lpiReleaseImage(&CConvolver_v->fmOut);
	lpiImage_lpiReleaseImage(&CConvolver_v->fm_fus1);
	lpiImage_lpiReleaseImage(&CConvolver_v->fm_fus2);

}





void CConvolver_Convolve(struct lpiImage *input, float *kernel, float bias, int kernel_dim, struct lpiImage *output)
{
	int i, j, k, l;
	int width=input->width;
	int height=input->height;
	//int outWidth = width - kernel_dim + 1;
	//int outHeight = height - kernel_dim + 1;

	float *pfm=(float *)input->imageData;
	float *pout=(float *)output->imageData;

	int wfm=input->width;
	int wout=output->width;

	int ws=kernel_dim/2;
	//premultiplications -> see commented lines below
	int displ = ws*kernel_dim + ws;
	int displ2 = -ws*wout -ws;

	if ((ws != 1) && (ws != 2)) {
		printf("ws = %d\n", ws);
		exit(1);
	}

	/* ws = 1 or 2 */
	for(i=ws;i<height-ws;i++) {
		int i_wout_displ2 = i*wout + displ2;
		float *pout1 = &pout[i_wout_displ2];

		for(j=ws;j<width-ws;j++)
		{
			register float product=bias;
			//for every weight of the conv. kernel
			if (ws == 1) {	/* kernel_dim = 3 */
				int i_wfm_j = i*wfm+j;
				int kk_displ;
				int i_k_wfm_j;
				float *pfm1; 
				float *kernel1;
//				k= - 1;
				{
					kk_displ = (-3)+displ;
					i_k_wfm_j = i_wfm_j-wfm;
					pfm1 = &pfm[i_k_wfm_j];
					kernel1 = &kernel[kk_displ];
					product += kernel1[-1] * pfm1[-1] + kernel1[0] * pfm1[0] + kernel1[1] * pfm1[1] ;
				}
//				k = 0;
				{
					kk_displ = displ;
					i_k_wfm_j = i_wfm_j;
					pfm1 = &pfm[i_k_wfm_j];
					kernel1 = &kernel[kk_displ];
					product += kernel1[-1] * pfm1[-1] + kernel1[0] * pfm1[0] + kernel1[1] * pfm1[1] ;
				}
//				k = 1;
				{
					kk_displ = (3)+displ;
					i_k_wfm_j = i_wfm_j+wfm;
					pfm1 = &pfm[i_k_wfm_j];
					kernel1 = &kernel[kk_displ];
					product += kernel1[-1] * pfm1[-1] + kernel1[0] * pfm1[0] + kernel1[1] * pfm1[1] ;
				}

			}
			else if (ws == 2)	/* kernel_dim = 5 */
			{
				int i_wfm_j = i*wfm+j;
				int kk_displ;
				int i_k_wfm_j;
				float *pfm1;
				float *kernel1;
//				k=-2;
				{
					kk_displ = (-10)+displ;
					i_k_wfm_j = i_wfm_j-2*wfm;
					pfm1 = &pfm[i_k_wfm_j];
					kernel1 = &kernel[kk_displ];
					product += kernel1[-2] * pfm1[-2] + kernel1[-1] * pfm1[-1] + kernel1[0] * pfm1[0]
						   + kernel1[1] * pfm1[1] + kernel1[2] * pfm1[2];
				}
//				k=-1;
				{
					kk_displ = (-5)+displ;
					i_k_wfm_j = i_wfm_j-wfm;
					pfm1 = &pfm[i_k_wfm_j];
					kernel1 = &kernel[kk_displ];
					product += kernel1[-2] * pfm1[-2] + kernel1[-1] * pfm1[-1] + kernel1[0] * pfm1[0]
						   + kernel1[1] * pfm1[1] + kernel1[2] * pfm1[2];
				}
//				k=0;
				{
					kk_displ = displ;
					i_k_wfm_j = i_wfm_j;
					pfm1 = &pfm[i_k_wfm_j];
					kernel1 = &kernel[kk_displ];
					product += kernel1[-2] * pfm1[-2] + kernel1[-1] * pfm1[-1] + kernel1[0] * pfm1[0]
						   + kernel1[1] * pfm1[1] + kernel1[2] * pfm1[2];
				}
//				k=1;
				{
					kk_displ = (5)+displ;
					i_k_wfm_j = i_wfm_j+wfm;
					pfm1 = &pfm[i_k_wfm_j];
					kernel1 = &kernel[kk_displ];
					product += kernel1[-2] * pfm1[-2] + kernel1[-1] * pfm1[-1] + kernel1[0] * pfm1[0]
						   + kernel1[1] * pfm1[1] + kernel1[2] * pfm1[2];
				}
//				k=2;
				{
					kk_displ = (10)+displ;
					i_k_wfm_j = i_wfm_j+2*wfm;
					pfm1 = &pfm[i_k_wfm_j];
					kernel1 = &kernel[kk_displ];
					product += kernel1[-2] * pfm1[-2] + kernel1[-1] * pfm1[-1] + kernel1[0] * pfm1[0]
						   + kernel1[1] * pfm1[1] + kernel1[2] * pfm1[2];
				}

			}
			else {
				for(k=-ws;k<=ws;k++) {
					int kk_displ = k*kernel_dim+displ;
					int i_k_wfm_j = (i+k)*wfm+j;
					float *pfm1 = &pfm[i_k_wfm_j];
					float *kernel1 = &kernel[kk_displ];
					for(l=-ws;l<=ws;l++)
					{
						product += kernel1[l] * pfm1[l];
					}
				}
			}

		//	pout[ (i-ws)*wout + j-ws ] = product;
		//	pout[ i_wout_displ2 + j] = product;
			pout1[j] = product;
		}
	}
}



//#define MULTIASSIGN_2

//same as Convolve, but it takes two input images and two kernels, fuses the results to output
void CConvolver_Convolve2(struct lpiImage *input, struct lpiImage *input2, float *kernel1, float *kernel2,
	float bias, int kernel_dim, struct lpiImage *output)
{
	int i, j, k, l;
	int width=input->width;
	int height=input->height;
	//int outWidth = width - kernel_dim + 1;
	//int outHeight = height - kernel_dim + 1;

	float *pfm=(float *)input->imageData;
	float *pout=(float *)output->imageData;
	float *pfm2=(float *)input2->imageData;

	int wfm=input->width;
	int wout=output->width;

	int ws=kernel_dim/2;
	//premultiplications -> see commented lines below
	int displ = ws*kernel_dim + ws;
	int displ2 = -ws*wout -ws;

	if ((ws != 1) && (ws != 2)) {
		printf("ws = %d\n", ws);
		exit(1);
	}

	for(i=ws;i<height-ws;i++) {
		int i_wout_displ2 = i*wout + displ2;
		float *pout1 = &pout[i_wout_displ2];

		for(j=ws;j<width-ws;j++)
		{
			float product=bias;
			//for every weight of the conv. kernel
			if (ws == 1)	/* kernel_dim = 3 */
			{
				int i_wfm_j = i*wfm+j;
				int i_k_wfm_j;
				float *pfm1;
				float *pfm21;
				int kk_displ;
				float *kernel11;
				float *kernel21;

//				k=-1;
				{
					i_k_wfm_j = i_wfm_j - wfm;
					pfm1 = &pfm[i_k_wfm_j];
					pfm21 = &pfm2[i_k_wfm_j];
					kk_displ = (-3) + displ;
					kernel11 = &kernel1[kk_displ];
					kernel21 = &kernel2[kk_displ];
#ifdef MULTIASSIGN_2
					product += kernel11[-1] * pfm1[-1] + kernel21[-1] * pfm21[-1];
					product	+= kernel11[0] * pfm1[0] + kernel21[0] * pfm21[0];
					product += kernel11[1] * pfm1[1] + kernel21[1] * pfm21[1];
#else
					product += kernel11[-1] * pfm1[-1] + kernel21[-1] * pfm21[-1]
							+ kernel11[0] * pfm1[0] + kernel21[0] * pfm21[0]
							+ kernel11[1] * pfm1[1] + kernel21[1] * pfm21[1];
#endif
				}
//				k=0;
				{
					i_k_wfm_j = i_wfm_j;
					pfm1 = &pfm[i_k_wfm_j];
					pfm21 = &pfm2[i_k_wfm_j];
					kk_displ = displ;
					kernel11 = &kernel1[kk_displ];
					kernel21 = &kernel2[kk_displ];
#ifdef MULTIASSIGN_2
					product += kernel11[-1] * pfm1[-1] + kernel21[-1] * pfm21[-1];
					product	+= kernel11[0] * pfm1[0] + kernel21[0] * pfm21[0];
					product += kernel11[1] * pfm1[1] + kernel21[1] * pfm21[1];
#else
					product += kernel11[-1] * pfm1[-1] + kernel21[-1] * pfm21[-1]
							+ kernel11[0] * pfm1[0] + kernel21[0] * pfm21[0]
							+ kernel11[1] * pfm1[1] + kernel21[1] * pfm21[1];
#endif
				}
//				k=1;
				{
					i_k_wfm_j = i_wfm_j + wfm;
					pfm1 = &pfm[i_k_wfm_j];
					pfm21 = &pfm2[i_k_wfm_j];
					kk_displ = (3) + displ;
					kernel11 = &kernel1[kk_displ];
					kernel21 = &kernel2[kk_displ];
#ifdef MULTIASSIGN_2
					product += kernel11[-1] * pfm1[-1] + kernel21[-1] * pfm21[-1];
					product	+= kernel11[0] * pfm1[0] + kernel21[0] * pfm21[0];
					product += kernel11[1] * pfm1[1] + kernel21[1] * pfm21[1];
#else
					product += kernel11[-1] * pfm1[-1] + kernel21[-1] * pfm21[-1]
							+ kernel11[0] * pfm1[0] + kernel21[0] * pfm21[0]
							+ kernel11[1] * pfm1[1] + kernel21[1] * pfm21[1];
#endif
				}

			}
			else if (ws == 2)
			{
				printf("!!! ws = %d\n", ws);

				for(k=-ws;k<=ws;k++) {
					int i_k_wfm_j = (i+k)*wfm+j;
					float *pfm1 = &pfm[i_k_wfm_j];
					float *pfm21 = &pfm2[i_k_wfm_j];
					int kk_displ = k*kernel_dim + displ;
					float *kernel11 = &kernel1[kk_displ];
					float *kernel21 = &kernel2[kk_displ];
					for(l=-ws;l<=ws;l++)
					{
						product += kernel11[l] * pfm1[l];
						product += kernel21[l] * pfm21[l];
					}
				}
			}
			else
			{
				for(k=-ws;k<=ws;k++) {
					int i_k_wfm_j = (i+k)*wfm+j;
					float *pfm1 = &pfm[i_k_wfm_j];
					float *pfm21 = &pfm2[i_k_wfm_j];
					int kk_displ = k*kernel_dim + displ;
					float *kernel11 = &kernel1[kk_displ];
					float *kernel21 = &kernel2[kk_displ];
					for(l=-ws;l<=ws;l++)
					{
						product += kernel11[l] * pfm1[l];
						product += kernel21[l] * pfm21[l];
					}
				}
			}

		//	pout[ (i-ws)*wout + j-ws ] = product;
		//	pout[ i*wout + j + displ2 ] = product;
			pout1[j] = product;
		}
	}
}



void CConvolver_ConvolveOutput(struct lpiImage **fms, int nofFMS, float *weights, float bias,
	int width, int height, struct lpiImage *output)
{
	int i, j, k;
	float *pout=(float *)output->imageData;

	for(i=0;i<height;i++) {
		register int i_width = i*width;
		float *pout1 = &pout[i_width];
		for(j=0;j<width;j++)
		{
			float product=bias;

			for(k=0;k<nofFMS;k++)
			{
				float *pfm=(float *)fms[k]->imageData;
				product += pfm[i_width+j] * weights[k];
			}
			TANH_SIMPLE(product, pout1[j]);
		}
	}
}




float *CConvolver_ConvolveRoughlyStillImage(struct CConvolver *CConvolver_v, struct lpiImage *img, int width, int height, int *ww, int *hh, int *tt, int scale)
{
	int i;
	int current_width;
	int current_height;
	int total_size;
	float *pOut;

	/////////////////1st layer//////////////////////
	CConvolver_v->width=width;
	CConvolver_v->height=height;

	CConvolver_v->fm0Sub=(struct lpiImage **)calloc(4, sizeof(struct lpiImage *));

	current_width=(width-4)/2;
	current_height=(height-4)/2;

	//	fm0_tmp=CConvolver_CreateImage(width-4, height-4);

	//convolution + subsampling
	for(i=0;i<4;i++)
	{
		struct lpiImage *fm0_tmp;

		fm0_tmp=CConvolver_CreateImage(width-4, height-4);

		CConvolver_v->fm0Sub[i]=CConvolver_CreateImage(current_width, current_height);

		CConvolver_Convolve(img, CConvolver_v->m_cnn->m_kernels0[i].kern, CConvolver_v->m_cnn->m_kernels0[i].bias, 5, fm0_tmp);

		CConvolver_SubSample(fm0_tmp, CConvolver_v->m_cnn->m_kernels0[i].coeff, CConvolver_v->m_cnn->m_kernels0[i].sbias, 0, CConvolver_v->fm0Sub[i]);

		lpiImage_lpiReleaseImage(&fm0_tmp);
	}


	///////////// PREPARE 2nd LAYER /////////////////////////

	CConvolver_v->fm1In=(struct lpiImage **)calloc(20, sizeof(struct lpiImage *));
	//	fm1=(lpiImage **)calloc(14, sizeof(lpiImage *));
	CConvolver_v->fm1Sub=(struct lpiImage **)calloc(14, sizeof(struct lpiImage *));

	for(i=0;i<8;i++)
		CConvolver_v->fm1In[i]=CConvolver_v->fm0Sub[i/2];


	CConvolver_v->fm1In[8]=CConvolver_v->fm0Sub[0];
	CConvolver_v->fm1In[9]=CConvolver_v->fm0Sub[0];
	CConvolver_v->fm1In[10]=CConvolver_v->fm0Sub[0];
	CConvolver_v->fm1In[11]=CConvolver_v->fm0Sub[1];
	CConvolver_v->fm1In[12]=CConvolver_v->fm0Sub[1];
	CConvolver_v->fm1In[13]=CConvolver_v->fm0Sub[2];

	CConvolver_v->fm1In[14]=CConvolver_v->fm0Sub[1];
	CConvolver_v->fm1In[15]=CConvolver_v->fm0Sub[2];
	CConvolver_v->fm1In[16]=CConvolver_v->fm0Sub[3];
	CConvolver_v->fm1In[17]=CConvolver_v->fm0Sub[2];
	CConvolver_v->fm1In[18]=CConvolver_v->fm0Sub[3];
	CConvolver_v->fm1In[19]=CConvolver_v->fm0Sub[3];

	////////////////////2nd layer//////////////////////

	//	fm1_tmp=CConvolver_CreateImage(current_width-2, current_height-2);
	//convolution
	for(i=0;i<8;i++)
	{
		struct lpiImage *fm1_tmp;
		fm1_tmp=CConvolver_CreateImage(current_width-2, current_height-2);

		CConvolver_v->fm1Sub[i]=CConvolver_CreateImage((current_width-2)/2, (current_height-2)/2);

		CConvolver_Convolve(CConvolver_v->fm1In[i], CConvolver_v->m_cnn->m_kernels1[i].kern, CConvolver_v->m_cnn->m_kernels1[i].bias, 3, fm1_tmp);

		CConvolver_SubSample(fm1_tmp, CConvolver_v->m_cnn->m_kernels1[i].coeff, CConvolver_v->m_cnn->m_kernels1[i].sbias, 0, CConvolver_v->fm1Sub[i]);

		lpiImage_lpiReleaseImage(&fm1_tmp);
	}

	for(i=8;i<14;i++)
	{
		struct lpiImage *fm1_tmp;
		fm1_tmp=CConvolver_CreateImage(current_width-2, current_height-2);

		CConvolver_v->fm1Sub[i]=CConvolver_CreateImage((current_width-2)/2, (current_height-2)/2);

		CConvolver_Convolve2(CConvolver_v->fm1In[i], CConvolver_v->fm1In[i+6], CConvolver_v->m_cnn->m_kernels1[i].kern, CConvolver_v->m_cnn->m_kernels1[i].kern2,
			CConvolver_v->m_cnn->m_kernels1[i].bias, 3, fm1_tmp);
		CConvolver_SubSample(fm1_tmp, CConvolver_v->m_cnn->m_kernels1[i].coeff, CConvolver_v->m_cnn->m_kernels1[i].sbias, 0, CConvolver_v->fm1Sub[i]);

		lpiImage_lpiReleaseImage(&fm1_tmp);

	}


	for(i=0;i<4;i++)
	{
		lpiImage_lpiReleaseImage(&CConvolver_v->fm0Sub[i]);
	}
	free(CConvolver_v->fm0Sub);
	free(CConvolver_v->fm1In);


	current_width=(current_width-2)/2;
	current_height=(current_height-2)/2;

	///////////////////3rd layer //////////////////////////////////////////////

	current_width = current_width - 6 + 1;
	current_height = current_height - 7 + 1;
	CConvolver_v->fm2=(struct lpiImage **)calloc(14, sizeof(struct lpiImage *));

	for(i=0;i<14;i++)
	{
		CConvolver_v->fm2[i]=CConvolver_CreateImage(current_width, current_height);

		CConvolver_ConvolveNeuron(CConvolver_v, CConvolver_v->fm1Sub[i], i, CConvolver_v->fm2[i]);

		lpiImage_lpiReleaseImage(&CConvolver_v->fm1Sub[i]);
	}
	free(CConvolver_v->fm1Sub);

	/////////////////  OUTPUT LAYER !!!   //////////////////

	CConvolver_v->fmOut=CConvolver_CreateImage(current_width, current_height);

	CConvolver_ConvolveOutput(CConvolver_v->fm2, 14, CConvolver_v->m_cnn->m_kernels3[0].kern, CConvolver_v->m_cnn->m_kernels3[0].bias,
		current_width, current_height, CConvolver_v->fmOut);

	for(i=0;i<14;i++)
	{
		lpiImage_lpiReleaseImage(&CConvolver_v->fm2[i]);
	}
	free(CConvolver_v->fm2);

	total_size=current_width*current_height;
	*tt=total_size;
	*hh=current_height;
	*ww=current_width;

	pOut=(float *)CConvolver_v->fmOut->imageData;
	return(pOut);
}

void CConvolver_DeallocateOutput(struct CConvolver *CConvolver_v)
{
	lpiImage_lpiReleaseImage(&CConvolver_v->fmOut);
}

struct lpiImage * CConvolver_CreateImage8U(int width, int height)
{
	struct lpiImage *img=lpiImage_lpiCreateImage( width, height, sizeof(unsigned char));
	return(img);
}
