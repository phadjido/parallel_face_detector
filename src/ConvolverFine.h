// ConvolverFine.h: interface for the CConvolverFine class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _CONVOLVER_FINE_H_
#define _CONVOLVER_FINE_H_

#include "NN.h"

#include "Convolver.h"

//const float c_lb=(float)lecunB;
//const float c_la=(float)lecunA;

struct CConvolverFine
{
	int m_width;
	int m_height;
	struct CNN *m_cnn;
};
	
void CConvolverFine_SetCNN(struct CConvolverFine *CConvolverFine_v, struct CNN *cnn);

float *CConvolverFine_ConvolveFine(struct CConvolverFine *CConvolverFine_v, struct lpiImage *img, float *res);

void CConvolverFine_SecondLayer(struct CConvolverFine *CConvolverFine_v, struct lpiImage **fmIn, int dispx, int dispy, float *res);
void CConvolverFine_NeuronLayer(struct CConvolverFine *CConvolverFine_v, struct lpiImage **fm, int dispx1, int dispy1, int dispx2, int dispy2, float *res);

void CConvolverFine_ConvolveNeuron(struct CConvolverFine *CConvolverFine_v, struct lpiImage *fm, int index, struct lpiImage *out);
void CConvolverFine_SubSample(struct lpiImage *fm, float coeff, float bias, int disp, int x_disp, int y_disp, struct lpiImage *sub);
struct lpiImage *CConvolverFine_CreateImage(int width, int height);

#endif


