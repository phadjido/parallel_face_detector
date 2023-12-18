#ifndef _MEM_H_
#define _MEM_H_


#include<stdio.h>



#include "MemUtils.h"

void InitMEM();



double **mem_getKernel_1(int level, int index, int *size, double *bias, int index2 /*=0*/);

void mem_getSubSample(int level, int index, double *coeff, double *bias);


double **mem_getKernel_2(int level, int index, int *width, int *height, double *bias);

double *mem_getOutputKernel(int level);

#endif

