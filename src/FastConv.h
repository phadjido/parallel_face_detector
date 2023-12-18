#ifndef _FASTCONV_H
#define _FASTCONV_H

#include "NN.h"

	//returns the array with the answers of the new in each image pixel
	//NOTE: in the right and the bottom part of the array there will be some
	//points not filled if the dimensions of the image do not feet the input dimensions of the net 
void convolveFine(struct CNN *cnn, double **input, int width, int height, double **res);


	//same as before, but returns the answers only in positions 4*i, 4*j
	// --> faster !
void convolveRoughly(struct CNN *cnn, double **input, int width, int height, double **res);

#endif

