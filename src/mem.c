
#include "mem.h"
#include "memDecl.h"


static double **allFMKernels[2][20];
static double allFMBias[2][14];
static double allFMSubBias[2][14];
static double allFMSubCoeff[2][14];
static double **allNeurons2[14];
static double allN2Bias[14];



void InitMEM()
{

/////////////////////

#include"memValues.h"

////////////////////


	allFMKernels[0][0]=mem_fm_0_0_kernel;
	allFMKernels[0][1]=mem_fm_0_1_kernel;
	allFMKernels[0][2]=mem_fm_0_2_kernel;
	allFMKernels[0][3]=mem_fm_0_3_kernel;

	allFMKernels[1][0]=mem_fm_1_0_kernel;
	allFMKernels[1][1]=mem_fm_1_1_kernel;
	allFMKernels[1][2]=mem_fm_1_2_kernel;
	allFMKernels[1][3]=mem_fm_1_3_kernel;
	allFMKernels[1][4]=mem_fm_1_4_kernel;
	allFMKernels[1][5]=mem_fm_1_5_kernel;
	allFMKernels[1][6]=mem_fm_1_6_kernel;
	allFMKernels[1][7]=mem_fm_1_7_kernel;
	allFMKernels[1][8]=mem_fm_1_8_kernel;
	allFMKernels[1][9]=mem_fm_1_9_kernel;
	allFMKernels[1][10]=mem_fm_1_10_kernel;
	allFMKernels[1][11]=mem_fm_1_11_kernel;
	allFMKernels[1][12]=mem_fm_1_12_kernel;
	allFMKernels[1][13]=mem_fm_1_13_kernel;

	allFMKernels[1][14]=mem_fm_1_8_1_kernel;
	allFMKernels[1][15]=mem_fm_1_9_1_kernel;
	allFMKernels[1][16]=mem_fm_1_10_1_kernel;
	allFMKernels[1][17]=mem_fm_1_11_1_kernel;
	allFMKernels[1][18]=mem_fm_1_12_1_kernel;
	allFMKernels[1][19]=mem_fm_1_13_1_kernel;


////////////////////////////////////////////////////

	allFMBias[0][0]=mem_fm_0_0_bias;
	allFMBias[0][1]=mem_fm_0_1_bias;
	allFMBias[0][2]=mem_fm_0_2_bias;
	allFMBias[0][3]=mem_fm_0_3_bias;

	allFMBias[1][0]=mem_fm_1_0_bias;
	allFMBias[1][1]=mem_fm_1_1_bias;
	allFMBias[1][2]=mem_fm_1_2_bias;
	allFMBias[1][3]=mem_fm_1_3_bias;
	allFMBias[1][4]=mem_fm_1_4_bias;
	allFMBias[1][5]=mem_fm_1_5_bias;
	allFMBias[1][6]=mem_fm_1_6_bias;
	allFMBias[1][7]=mem_fm_1_7_bias;
	allFMBias[1][8]=mem_fm_1_8_bias;
	allFMBias[1][9]=mem_fm_1_9_bias;
	allFMBias[1][10]=mem_fm_1_10_bias;
	allFMBias[1][11]=mem_fm_1_11_bias;
	allFMBias[1][12]=mem_fm_1_12_bias;
	allFMBias[1][13]=mem_fm_1_13_bias;
//////////////////////////////////////////////////////

	allFMSubBias[0][0]=mem_fm_0_0_subbias;
	allFMSubBias[0][1]=mem_fm_0_1_subbias;
	allFMSubBias[0][2]=mem_fm_0_2_subbias;
	allFMSubBias[0][3]=mem_fm_0_3_subbias;

	allFMSubBias[1][0]=mem_fm_1_0_subbias;
	allFMSubBias[1][1]=mem_fm_1_1_subbias;
	allFMSubBias[1][2]=mem_fm_1_2_subbias;
	allFMSubBias[1][3]=mem_fm_1_3_subbias;
	allFMSubBias[1][4]=mem_fm_1_4_subbias;
	allFMSubBias[1][5]=mem_fm_1_5_subbias;
	allFMSubBias[1][6]=mem_fm_1_6_subbias;
	allFMSubBias[1][7]=mem_fm_1_7_subbias;
	allFMSubBias[1][8]=mem_fm_1_8_subbias;
	allFMSubBias[1][9]=mem_fm_1_9_subbias;
	allFMSubBias[1][10]=mem_fm_1_10_subbias;
	allFMSubBias[1][11]=mem_fm_1_11_subbias;
	allFMSubBias[1][12]=mem_fm_1_12_subbias;
	allFMSubBias[1][13]=mem_fm_1_13_subbias;
//////////////////////////////////////////////////////

	allFMSubCoeff[0][0]=mem_fm_0_0_subCoeff;
	allFMSubCoeff[0][1]=mem_fm_0_1_subCoeff;
	allFMSubCoeff[0][2]=mem_fm_0_2_subCoeff;
	allFMSubCoeff[0][3]=mem_fm_0_3_subCoeff;

	allFMSubCoeff[1][0]=mem_fm_1_0_subCoeff;
	allFMSubCoeff[1][1]=mem_fm_1_1_subCoeff;
	allFMSubCoeff[1][2]=mem_fm_1_2_subCoeff;
	allFMSubCoeff[1][3]=mem_fm_1_3_subCoeff;
	allFMSubCoeff[1][4]=mem_fm_1_4_subCoeff;
	allFMSubCoeff[1][5]=mem_fm_1_5_subCoeff;
	allFMSubCoeff[1][6]=mem_fm_1_6_subCoeff;
	allFMSubCoeff[1][7]=mem_fm_1_7_subCoeff;
	allFMSubCoeff[1][8]=mem_fm_1_8_subCoeff;
	allFMSubCoeff[1][9]=mem_fm_1_9_subCoeff;
	allFMSubCoeff[1][10]=mem_fm_1_10_subCoeff;
	allFMSubCoeff[1][11]=mem_fm_1_11_subCoeff;
	allFMSubCoeff[1][12]=mem_fm_1_12_subCoeff;
	allFMSubCoeff[1][13]=mem_fm_1_13_subCoeff;
//////////////////////////////////////////////////////

	allNeurons2[0]=mem_n_2_0_kernel;
	allNeurons2[1]=mem_n_2_1_kernel;
	allNeurons2[2]=mem_n_2_2_kernel;
	allNeurons2[3]=mem_n_2_3_kernel;
	allNeurons2[4]=mem_n_2_4_kernel;
	allNeurons2[5]=mem_n_2_5_kernel;
	allNeurons2[6]=mem_n_2_6_kernel;
	allNeurons2[7]=mem_n_2_7_kernel;
	allNeurons2[8]=mem_n_2_8_kernel;
	allNeurons2[9]=mem_n_2_9_kernel;
	allNeurons2[10]=mem_n_2_10_kernel;
	allNeurons2[11]=mem_n_2_11_kernel;
	allNeurons2[12]=mem_n_2_12_kernel;
	allNeurons2[13]=mem_n_2_13_kernel;
//////////////////////////////////////////////////////

	allN2Bias[0]=mem_n_2_0_bias;
	allN2Bias[1]=mem_n_2_1_bias;
	allN2Bias[2]=mem_n_2_2_bias;
	allN2Bias[3]=mem_n_2_3_bias;
	allN2Bias[4]=mem_n_2_4_bias;
	allN2Bias[5]=mem_n_2_5_bias;
	allN2Bias[6]=mem_n_2_6_bias;
	allN2Bias[7]=mem_n_2_7_bias;
	allN2Bias[8]=mem_n_2_8_bias;
	allN2Bias[9]=mem_n_2_9_bias;
	allN2Bias[10]=mem_n_2_10_bias;
	allN2Bias[11]=mem_n_2_11_bias;
	allN2Bias[12]=mem_n_2_12_bias;
	allN2Bias[13]=mem_n_2_13_bias;
//////////////////////////////////////////////////////



}





double **mem_getKernel_1(int level, int index, int *size, double *bias, int index2)
{
		//HARDCODED FEATURE
	if(level==0)
		*size=5;
	else
		*size=3;

	*bias=allFMBias[level][index];

	if(index2==0)
		return(allFMKernels[level][index]);
	else
		return(allFMKernels[level][index+6]);
}



void mem_getSubSample(int level, int index, double *coeff, double *bias)
{
	*bias=allFMSubBias[level][index];
	*coeff=allFMSubCoeff[level][index];
}


double **mem_getKernel_2(int level, int index, int *width, int *height, double *bias)
{
		//HARDCODED FEATURE
	*width=6;
	*height=7;

	*bias=allN2Bias[index];

	return( allNeurons2[index] );
}


double *mem_getOutputKernel(int level)
{
	return( mem_n_3_kernel );
}
