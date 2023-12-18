#ifndef _MEMUTILS_H
#define _MEMUTILS_H

	//allocation - deallocation
	//of 1d - 2d matrices of int and double
	//(special useful the 2d cases !)

int *Alloc1DInt(int size);
double *Alloc1DDouble(int size);
float *Alloc1DFloat(int size);
int **Alloc2DInt(int width, int height);
double **Alloc2DDouble(int width, int height);
float **Alloc2DFloat(int width, int height);

void Del1D(void *array);
void Del1D_i(int *array);
void Del1D_d(double *array);
void Del1D_f(float *array);
void Del2D(int height, void **array);
void Del2D_i(int height, int **array);
void Del2D_d(int height, double **array);
void Del2D_f(int height, float **array);


#endif









