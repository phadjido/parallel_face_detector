// NN.h: interface for the CNN class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _NN_H_
#define _NN_H_

#include <stdlib.h>
#include "FeatureMap.h"
#include "Neuron.h"

#include "MemUtils.h"


//weights of the FMs of 1st and 2bd level
//1st layer: kernel: 5x5
//2nd: 3x3
struct ConvKernelFM
{
	float *kern;
	float bias;
	float coeff;
	float sbias;
	float *kern2;
};

struct ConvKernelNeuron
{
	float *kern;
	float bias;
};

struct CNN  
{
		//N1 layer weights:
		//kern = 6x7 = 42 weights
	struct ConvKernelFM *m_kernels0;
	struct ConvKernelFM *m_kernels1;
	struct ConvKernelNeuron *m_kernels2;
	struct ConvKernelNeuron *m_kernels3;


		//kernels, biases, coeffs, subbiases

	struct Neuron * m_N2;
	struct Neuron * m_N1;
	struct FeatureMap * m_FM2;
	struct FeatureMap * m_FM1;
	int m_nofLayers;
	int m_sizeInputX;
	int m_sizeInputY;
	int *m_LayerLength;
};


int CNN_readState(struct CNN *CNN_v, char *filename);
void CNN_CreateConvolveKernels(struct CNN *CNN_v);

void CNN_CreateConvKernelN(struct CNN *CNN_v, int index);
void CNN_CreateConvKernelFM(struct CNN *CNN_v, int level, int index);





//fast convolution stuff
//NOTE: some assumptions are made about the topology !!!

double **CNN_getKernel(struct CNN *CNN_v, int level, int index, int *size, double *bias);
void CNN_getSubSample(struct CNN *CNN_v, int level, int index, double *coeff, double *bias);
double **CNN_getNeuronKernel(struct CNN *CNN_v, int index, int *width, int *height, double *bias);
double *CNN_getOutputKernel(struct CNN *CNN_v);



#endif
