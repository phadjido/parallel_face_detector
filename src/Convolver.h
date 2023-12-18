// Convolver.h: interface for the CConvolver class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _CONVOLVER_H_
#define _CONVOLVER_H_


#include "NN.h"
#include "LpiImage.h"


//const float c_lb=lecunB; //(float)lecunB;
//const float c_la=lecunA; //(float)lecunA;

#define c_lb	(lecunB)
#define c_la	(lecunA)

struct CConvolver
{
	struct CNN *m_cnn;

	struct lpiImage **fm0;
	struct lpiImage **fm0Sub;
	struct lpiImage **fm1In;
	struct lpiImage **fm1;
	struct lpiImage **fm1Sub;
	struct lpiImage **fm2;
	struct lpiImage *fmOut;
	struct lpiImage *fm_fus1;
	struct lpiImage *fm_fus2;
	int width;
	int height;
};


void CConvolver_SetCNN(struct CConvolver *CConvolver_v, struct CNN *cnn);
float *CConvolver_ConvolveRoughly(struct CConvolver *CConvolver_v, struct lpiImage *img, int *ww, int *hh, int *tt);

struct lpiImage * CConvolver_CreateImage8U(int width, int height);
void CConvolver_DeallocateOutput(struct CConvolver *CConvolver_v);

int CConvolver_InitFMs(struct CConvolver *CConvolver_v, int width, int height);
void CConvolver_FreeFMs(struct CConvolver *CConvolver_v);

void CConvolver_ConvolveNeuron(struct CConvolver *CConvolver_v, struct lpiImage *fm, int index, struct lpiImage *out);
void CConvolver_SubSample(struct lpiImage *fm, float coeff, float bias, int disp, struct lpiImage *sub);
struct lpiImage *CConvolver_CreateImage(int width, int height);

float *CConvolver_ConvolveRoughlyStillImage(struct CConvolver *CConvolver_v, struct lpiImage *img, int width, int height, int *ww, int *hh, int *tt, int scale);

void CConvolver_Convolve(struct lpiImage *input, float *kernel, float bias, int kernel_dim, struct lpiImage *output);
void CConvolver_Convolve2(struct lpiImage *input, struct lpiImage *input2, float *kernel1, float *kernel2,
float bias, int kernel_dim, struct lpiImage *output);
void CConvolver_ConvolveOutput(struct lpiImage **fms, int nofFMS, float *weights, float bias,
int width, int height, struct lpiImage *output);



#endif
