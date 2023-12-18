#include<stdlib.h>
#include "MemUtils.h"

//#define calloc(x,y)	memalign(64,x*y)

int *Alloc1DInt(int size)
{
	int *res=(int *)calloc(size, sizeof(int));
	return(res);
}

double *Alloc1DDouble(int size)
{
	double *res=(double *)calloc(size, sizeof(double));
	return(res);
}
float *Alloc1DFloat(int size)
{
	float *res=(float *)calloc(size, sizeof(float));
	return(res);
}


int **Alloc2DInt(int width, int height)
{
	int i;
	int **res=(int **)calloc(height, sizeof(int));
	for(i=0;i<height;i++)
		res[i]=(int *)calloc(width, sizeof(int));
	return(res);
}
double **Alloc2DDouble(int width, int height)
{
	int i;
	double **res=(double **)calloc(height, sizeof(double));
	for(i=0;i<height;i++)
		res[i]=(double *)calloc(width, sizeof(double));
	return(res);
}

float **Alloc2DFloat(int width, int height)
{
	int i;
	float **res=(float **)calloc(height, sizeof(float));
	for(i=0;i<height;i++)
		res[i]=(float *)calloc(width, sizeof(float));
	return(res);
}


void Del1D(void *array)
{
	free(array);
}

void Del1D_i(int *array)
{
	free(array);
}

void Del1D_d(double *array)
{
	free(array);
}

void Del1D_f(float *array)
{
	free(array);
}


void Del2D(int height, void **array)
{
	int i;
	for(i=0;i<height;i++)
		free(array[i]);
	free(array);
}

void Del2D_i(int height, int **array)
{
	int i;
	for(i=0;i<height;i++)
		free(array[i]);
	free(array);
}

void Del2D_d(int height, double **array)
{
	int i;
	for(i=0;i<height;i++)
		free(array[i]);
	free(array);
}

void Del2D_f(int height, float **array)
{
	int i;
	for(i=0;i<height;i++)
		free(array[i]);
	free(array);
}
