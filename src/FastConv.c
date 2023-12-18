#include "FastConvUtils.h"
#include "FastConv.h"



//second layer of feature maps
void secondLayer(double ***fm0Sub, int dispX, int dispY, int width, int height, struct CNN *cnn, double **res);

//the two neurons layers
void NeuronLayer(double ***fm1Sub, int dispX1, int dispY1, int dispX2, int dispY2, int width, int height, struct CNN *cnn, double **res);
//TODO: add array deletion


//first layer
void convolveFine(struct CNN *cnn, double **input, int width, int height, double **res)
{
	//temporary variables
	double **kernel;
	double bias, subCoeff;
	//int c_width, c_height
	int c_size;
	//input dimensions for the current layer
	int iw=width, ih=height;
	//... and the output dimensions
	int ow, oh;
	int i=0, j;
	//char buf[256];

	double ***fm0=(double ***)calloc(4, sizeof(double **));

	//first level: various displacements for the subs
	//decond level: the four resp. feature maps produced
	//3rd-4th level: the 2D arrays of FMs
	double ****fm0Sub=(double ****)calloc(4, sizeof(double ***));
	for(i=0;i<4;i++)
		fm0Sub[i]=(double ***)calloc(4, sizeof(double **));

	////////////// FIRST LAYER /////////////////////////

	for(i=0;i<4;i++)
	{
		kernel=CNN_getKernel(cnn, 0, i, &c_size, &bias);
		fm0[i]=convolve(input, iw, ih, kernel, bias, c_size, &ow, &oh);

		CNN_getSubSample(cnn, 0, i, &subCoeff, &bias);
		fm0Sub[0][i]=subSample(fm0[i], ow, oh, subCoeff, bias, 0, 0);
		fm0Sub[1][i]=subSample(fm0[i], ow, oh, subCoeff, bias, 0, 1);
		fm0Sub[2][i]=subSample(fm0[i], ow, oh, subCoeff, bias, 1, 0);
		fm0Sub[3][i]=subSample(fm0[i], ow, oh, subCoeff, bias, 1, 1);
	}
	
	ih=oh/2;
	iw=ow/2;

	secondLayer(fm0Sub[0], 0, 0, iw, ih, cnn, res);
	secondLayer(fm0Sub[1], 0, 1, iw, ih, cnn, res);
	secondLayer(fm0Sub[2], 1, 0, iw, ih, cnn, res);
	secondLayer(fm0Sub[3], 1, 1, iw, ih, cnn, res);


	for(i=0;i<4;i++)
		Del2D(oh, (void *)fm0[i]);
	free(fm0);
	for(i=0;i<4;i++)
	{
		for(j=0;j<4;j++)
			Del2D(oh/2, (void *)fm0Sub[i][j]);
		free(fm0Sub[i]);
	}
	free(fm0Sub);

}


void secondLayer(double ***fm0Sub, int dispX, int dispY, int width, int height, struct CNN *cnn, double **res)
{
	//temporary variables
	double **kernel;
	double bias, subCoeff;
	//int c_width, c_height
	int c_size;
	//input dimensions for the current layer
	int iw=width, ih=height;
	//... and the output dimensions
	int ow, oh;
	int i=0, j;


	///////////// PREPARE SECOND LAYER (MIXING FMS)///////////////////////

	double **fm00_01=add(fm0Sub[0], fm0Sub[1], iw, ih);
	double **fm00_02=add(fm0Sub[0], fm0Sub[2], iw, ih);
	double **fm00_03=add(fm0Sub[0], fm0Sub[3], iw, ih);
	double **fm01_02=add(fm0Sub[1], fm0Sub[2], iw, ih);
	double **fm01_03=add(fm0Sub[1], fm0Sub[3], iw, ih);
	double **fm02_03=add(fm0Sub[2], fm0Sub[3], iw, ih);


	double ***fm1In=(double ***)calloc(14, sizeof(double **));
	double ***fm1=(double ***)calloc(14, sizeof(double **));
	double ****fm1Sub=(double ****)calloc(4, sizeof(double ***));
	for(i=0;i<4;i++)
		fm1Sub[i]=(double ***)calloc(14, sizeof(double **));

	fm1In[0]=fm0Sub[0];
	fm1In[1]=fm0Sub[0];
	fm1In[2]=fm0Sub[1];
	fm1In[3]=fm0Sub[1];
	fm1In[4]=fm0Sub[2];
	fm1In[5]=fm0Sub[2];
	fm1In[6]=fm0Sub[3];
	fm1In[7]=fm0Sub[3];
	fm1In[8]=fm00_01;
	fm1In[9]=fm00_02;
	fm1In[10]=fm00_03;
	fm1In[11]=fm01_02;
	fm1In[12]=fm01_03;
	fm1In[13]=fm02_03;


	//////// SECOND LAYER ///////////////////
	for(i=0;i<14;i++)
	{
		kernel=CNN_getKernel(cnn, 1, i, &c_size, &bias);
		fm1[i]=convolve(fm1In[i], iw, ih, kernel, bias, c_size, &ow, &oh);

		CNN_getSubSample(cnn, 1, i, &subCoeff, &bias);
		fm1Sub[0][i]=subSample(fm1[i], ow, oh, subCoeff, bias, 0, 0);
		fm1Sub[1][i]=subSample(fm1[i], ow, oh, subCoeff, bias, 0, 1);
		fm1Sub[2][i]=subSample(fm1[i], ow, oh, subCoeff, bias, 1, 0);
		fm1Sub[3][i]=subSample(fm1[i], ow, oh, subCoeff, bias, 1, 1);
	}


	ih=oh/2;
	iw=ow/2;

	NeuronLayer(fm1Sub[0], dispX, dispY, 0, 0, iw, ih, cnn, res);
	NeuronLayer(fm1Sub[1], dispX, dispY, 0, 1, iw, ih, cnn, res);
	NeuronLayer(fm1Sub[2], dispX, dispY, 1, 0, iw, ih, cnn, res);
	NeuronLayer(fm1Sub[3], dispX, dispY, 1, 1, iw, ih, cnn, res);


	Del2D(ih, (void *)fm00_01);
	Del2D(ih, (void *) fm00_02);
	Del2D(ih, (void *)fm00_03);
	Del2D(ih, (void *)fm01_02);
	Del2D(ih, (void *)fm01_03);
	Del2D(ih, (void *)fm02_03);
	

	for(i=0;i<14;i++)
		Del2D(oh, (void *)fm1[i]);
	free(fm1);
	for(i=0;i<4;i++)
	{
		for(j=0;j<14;j++)
			Del2D(oh/2, (void *)fm1Sub[i][j]);
		free(fm1Sub[i]);
	}
	free(fm1Sub);

}



void NeuronLayer(double ***fm1Sub, int dispX1, int dispY1, int dispX2, int dispY2, int width, int height, struct CNN *cnn, double **res)
{
	//temporary variables
	double **kernel;
	double bias;
	//double subCoeff;
	int c_width, c_height;
	//int c_size;
	//input dimensions for the current layer
	int iw=width, ih=height;
	//... and the output dimensions
	int ow, oh;
	int i=0, j;

	////////// THIRD LAYER (NEURONS HERE !)///////////////////////////////////////////

	double ***fm2=(double ***)calloc(14, sizeof(double **));

	double *outputWeights;
	double **FMO;

	for(i=0;i<14;i++)
	{
		kernel=CNN_getNeuronKernel(cnn, i, &c_width, &c_height, &bias);
		fm2[i]=convolveNeuron(fm1Sub[i], iw, ih, kernel, bias, c_width, c_height, &ow, &oh);
		Del2D(c_height, (void *)kernel);
	}

	outputWeights=CNN_getOutputKernel(cnn);
	FMO=convolveOutput(fm2, 14, outputWeights, ow, oh);

	//now put the final results into the array
	for(i=0;i<oh;i++)
		for(j=0;j<ow;j++)
			res[4*i + 2*dispY2 + dispY1][4*j + 2*dispX2 + dispX1] = FMO[i][j];

	for(i=0;i<14;i++)
		Del2D(oh, (void *)fm2[i]);
	free(fm2);
	Del2D(oh, (void *)FMO);

}





void convolveRoughly(struct CNN *cnn, double **input, int width, int height, double **res)
{
	//temporary variables
	double **kernel;
	double bias, subCoeff;
	int c_width, c_height, c_size;
	//input dimensions for the current layer
	int iw=width, ih=height;
	//... and the output dimensions
	int ow, oh;
	//char buf[256];
	int i=0, j;

	double ***fm0=(double ***)calloc(4, sizeof(double **));
	double ***fm0Sub=(double ***)calloc(4, sizeof(double **));


	double **fm00_01;
	double **fm00_02;
	double **fm00_03;
	double **fm01_02;
	double **fm01_03;
	double **fm02_03;

	double ***fm1;
	double ***fm1Sub;
	double ***fm1In;

	double ***fm2;

	double *outputWeights;
	double **FMO;

	////////////// FIRST LAYER /////////////////////////

	for(i=0;i<4;i++)
	{
		kernel=CNN_getKernel(cnn, 0, i, &c_size, &bias);
		fm0[i]=convolve(input, iw, ih, kernel, bias, c_size, &ow, &oh);

		CNN_getSubSample(cnn, 0, i, &subCoeff, &bias);
		fm0Sub[i]=subSample(fm0[i], ow, oh, subCoeff, bias, 0, 0);
	}
	
	ih=oh/2;
	iw=ow/2;



	///////////// PREPARE SECOND LAYER (MIXING FMS)///////////////////////

	fm00_01=add(fm0Sub[0], fm0Sub[1], iw, ih);
	fm00_02=add(fm0Sub[0], fm0Sub[2], iw, ih);
	fm00_03=add(fm0Sub[0], fm0Sub[3], iw, ih);
	fm01_02=add(fm0Sub[1], fm0Sub[2], iw, ih);
	fm01_03=add(fm0Sub[1], fm0Sub[3], iw, ih);
	fm02_03=add(fm0Sub[2], fm0Sub[3], iw, ih);


	fm1=(double ***)calloc(14, sizeof(double **));
	fm1Sub=(double ***)calloc(14, sizeof(double **));
	fm1In=(double ***)calloc(14, sizeof(double **));

	fm1In[0]=fm0Sub[0];
	fm1In[1]=fm0Sub[0];
	fm1In[2]=fm0Sub[1];
	fm1In[3]=fm0Sub[1];
	fm1In[4]=fm0Sub[2];
	fm1In[5]=fm0Sub[2];
	fm1In[6]=fm0Sub[3];
	fm1In[7]=fm0Sub[3];
	fm1In[8]=fm00_01;
	fm1In[9]=fm00_02;
	fm1In[10]=fm00_03;
	fm1In[11]=fm01_02;
	fm1In[12]=fm01_03;
	fm1In[13]=fm02_03;

	//////// SECOND LAYER ///////////////////
	for(i=0;i<14;i++)
	{
		kernel=CNN_getKernel(cnn, 1, i, &c_size, &bias);
		fm1[i]=convolve(fm1In[i], iw, ih, kernel, bias, c_size, &ow, &oh);

		CNN_getSubSample(cnn, 1, i, &subCoeff, &bias);
		fm1Sub[i]=subSample(fm1[i], ow, oh, subCoeff, bias, 0, 0);
	}


	////////// THIRD LAYER (NEURONS HERE !)///////////////////////////////////////////
	iw=ow/2;
	ih=oh/2;
	fm2=(double ***)calloc(14, sizeof(double **));

	for(i=0;i<14;i++)
	{
		kernel=CNN_getNeuronKernel(cnn, i, &c_width, &c_height, &bias);
		fm2[i]=convolveNeuron(fm1Sub[i], iw, ih, kernel, bias, c_width, c_height, &ow, &oh);
	}

	outputWeights=CNN_getOutputKernel(cnn);
	FMO=convolveOutput(fm2, 14, outputWeights, ow, oh);

	for(i=0;i<oh;i++)
		for(j=0;j<ow;j++)
			res[4*i][4*j]=FMO[i][j];


}