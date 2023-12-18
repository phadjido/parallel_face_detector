#ifndef _FASTCONVUTILS_H
#define _FASTCONVUTILS_H

#include "NN.h"


	//mask - convolve
double **convolve(double **input, int width, int height, double **kernel, double bias,
       int kernel_dim, int *ow, int *oh);

	//subsampling (note the displacements)
double **subSample(double **input, int width, int height, double coeff, double bias, int x_disp, int y_disp);

	//FMs addition
double **add(double **input1, double **input2, int width, int height);

	//convolve with the weights of a neuron
double **convolveNeuron(double **input, int width, int height,
	double **kernel, double bias, int kernel_width, int kernel_height, int *ow, int *oh);

	//output fusion
double **convolveOutput(double ***fms, int nofFMS, double *weights, int width, int height);



#endif


