// NN.cpp: implementation of the CNN class.
//
//////////////////////////////////////////////////////////////////////

#include "NN.h"
#include "mem.h"

#include <stdio.h>
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////



//CNN::CNN()
//{
//
//}

//CNN::~CNN()
//{
//
//}

int CNN_readState(struct CNN *CNN_v, char *filename)
{
//	AfxMessageBox("NOT USED ANY MORE");
	FILE *fp;
	char buf[512];
	char buf1[256], buf2[256];
	int i=0, j;

	return(0);

	fp=fopen(filename, "r");
	if(fp==NULL)
	{
//		AfxMessageBox("Cannot open file [s] for reading !\n");
		return(0);
	}
		//ignore comments
	fgets(buf, sizeof(buf), fp);
		//read some member variables
	fgets(buf, sizeof(buf), fp);
	sscanf(buf, "%d %d %d", &CNN_v->m_nofLayers, &CNN_v->m_sizeInputX, &CNN_v->m_sizeInputY);

	CNN_v->m_LayerLength=(int *)malloc(sizeof(int)*CNN_v->m_nofLayers);

	
	i=0;
	fgets(buf, sizeof(buf), fp);
	sscanf(buf, "%d", &(CNN_v->m_LayerLength[i]));
	CNN_v->m_FM1 = (struct FeatureMap *)malloc(sizeof(struct FeatureMap)*CNN_v->m_LayerLength[i]);


	i=1;
	fgets(buf, sizeof(buf), fp);
	sscanf(buf, "%d", &(CNN_v->m_LayerLength[i]));
	CNN_v->m_FM2 = (struct FeatureMap *)malloc(sizeof(struct FeatureMap)*CNN_v->m_LayerLength[i]);


	i=2;
	fgets(buf, sizeof(buf), fp);
	sscanf(buf, "%d", &(CNN_v->m_LayerLength[i]));
	CNN_v->m_N1 = (struct Neuron *)malloc(sizeof(struct Neuron)*CNN_v->m_LayerLength[i]);



	i=3;
	fgets(buf, sizeof(buf), fp);
	sscanf(buf, "%d", &(CNN_v->m_LayerLength[i]));
	CNN_v->m_N2 = (struct Neuron *)malloc(sizeof(struct Neuron)*CNN_v->m_LayerLength[i]);

/////////////////////////////////////////////////

	i=0;
	for(j=0;j<CNN_v->m_LayerLength[i];j++)
	{
		fgets(buf, sizeof(buf), fp);
		sscanf(buf, "%s %s\n", buf1, buf2);
		FeatureMap_readState(&CNN_v->m_FM1[j], fp);
	}


	i=1;
	for(j=0;j<CNN_v->m_LayerLength[i];j++)
	{
		fgets(buf, sizeof(buf), fp);
		sscanf(buf, "%s %s\n", buf1, buf2);
		FeatureMap_readState(&CNN_v->m_FM2[j], fp);
	}

	i=2;
	for(j=0;j<CNN_v->m_LayerLength[i];j++)
	{
		fgets(buf, sizeof(buf), fp);
		sscanf(buf, "%s %s\n", buf1, buf2);
		Neuron_readState(&CNN_v->m_N1[j], fp);
	}

	i=3;
	for(j=0;j<CNN_v->m_LayerLength[i];j++)
	{
		fgets(buf, sizeof(buf), fp);
		sscanf(buf, "%s %s\n", buf1, buf2);
		Neuron_readState(&CNN_v->m_N2[j], fp);
	}

	fclose(fp);

	return(1);
}

void CNN_CreateConvolveKernels(struct CNN *CNN_v)
{
	int i, j, k;
	double *weights;

	InitMEM();

	CNN_v->m_kernels0=(struct ConvKernelFM *)malloc(sizeof(struct ConvKernelFM)*4);
	for(i=0;i<4;i++)
		CNN_CreateConvKernelFM(CNN_v, 0, i);

	CNN_v->m_kernels1=(struct ConvKernelFM *)malloc(sizeof(struct ConvKernelFM)*14);
	for(j=0;j<14;j++)
		CNN_CreateConvKernelFM(CNN_v, 1, j);

	CNN_v->m_kernels2=(struct ConvKernelNeuron *)malloc(sizeof(struct ConvKernelNeuron)*14);
	for(k=0;k<14;k++)
		CNN_CreateConvKernelN(CNN_v, k);



	CNN_v->m_kernels3=(struct ConvKernelNeuron *)malloc(sizeof(struct ConvKernelNeuron)*1);
	weights=mem_getOutputKernel(3);

	CNN_v->m_kernels3[0].bias=(float)weights[0];
	CNN_v->m_kernels3[0].kern=(float *)calloc(14, sizeof(float));
	for(i=1;i<15;i++)
		CNN_v->m_kernels3[0].kern[i-1]=(float)weights[i];

}

void CNN_CreateConvKernelFM(struct CNN *CNN_v, int level, int index)
{
	int i, j;

	if(level==0)
	{
		int size;
		double bias;
		double coeff, sbias;
		double **k;
		int count=0;

		k=mem_getKernel_1(0, index, &size, &bias, 0);
		mem_getSubSample(0, index, &coeff, &sbias);

		CNN_v->m_kernels0[index].kern=(float *)calloc(25, sizeof(float));
		count=0;
		for(i=0;i<5;i++)
			for(j=0;j<5;j++)
				CNN_v->m_kernels0[index].kern[count++]=(float)k[i][j];

		CNN_v->m_kernels0[index].bias = (float)bias;
		CNN_v->m_kernels0[index].sbias = (float)sbias;
		CNN_v->m_kernels0[index].coeff = (float)coeff;
		CNN_v->m_kernels0[index].kern2 = NULL;
	}
	else /* if(level==1) */
	{
		int size;
		double bias;
		double coeff, sbias;
		double **k;
		int count=0;

		k=mem_getKernel_1(1, index, &size, &bias, 0);
		mem_getSubSample(1, index, &coeff, &sbias);

		CNN_v->m_kernels1[index].kern=(float *)calloc(9, sizeof(float));
		count=0;
		for(i=0;i<3;i++)
			for(j=0;j<3;j++)
				CNN_v->m_kernels1[index].kern[count++]=(float)k[i][j];

		CNN_v->m_kernels1[index].bias = (float)bias;
		CNN_v->m_kernels1[index].sbias = (float)sbias;
		CNN_v->m_kernels1[index].coeff = (float)coeff;


			//ADDITION FOR THE NEW CODE WITH TWO KERNEL FOR SOME FMS
		if(index<8)
		{
			CNN_v->m_kernels1[index].kern2 = NULL;
			return;
		}
		k=mem_getKernel_1(1, index, &size, &bias, 1);

		CNN_v->m_kernels1[index].kern2=(float *)calloc(9, sizeof(float));
		count=0;
		for(i=0;i<3;i++)
			for(j=0;j<3;j++)
				CNN_v->m_kernels1[index].kern2[count++]=(float)k[i][j];
	}
}

void CNN_CreateConvKernelN(struct CNN *CNN_v, int index)
{
	int i, j;
	int w, h;
	double bias;
	double **nk=mem_getKernel_2(2, index, &w, &h, &bias);
	int count;

	CNN_v->m_kernels2[index].bias=(float)bias;
	CNN_v->m_kernels2[index].kern=(float *)calloc(42, sizeof(float));
	count=0;
	for(i=0;i<h;i++)
		for(j=0;j<w;j++)
			CNN_v->m_kernels2[index].kern[count++]=(float)nk[i][j];

}


double **CNN_getKernel(struct CNN *CNN_v, int level, int index, int *size, double *bias)
{
	if(level==0)
		return( FeatureMap_getKernel(&CNN_v->m_FM1[index], size, bias) );
	else
		return( FeatureMap_getKernel(&CNN_v->m_FM2[index], size, bias) );
}

void CNN_getSubSample(struct CNN *CNN_v, int level, int index, double *coeff, double *bias)
{
	if(level==0)
		FeatureMap_getSubSample(&CNN_v->m_FM1[index], coeff, bias);
	else
		FeatureMap_getSubSample(&CNN_v->m_FM2[index], coeff, bias);
}

double **CNN_getNeuronKernel(struct CNN *CNN_v, int index, int *width, int *height, double *bias)
{
	int i, j;
	double *weights=Neuron_getWeights(&CNN_v->m_N1[index]);
	int w=6, h=7;
	double **kernel=Alloc2DDouble(w, h);
	int count=1;

	*bias=weights[0];
	for(i=0;i<h;i++)
		for(j=0;j<w;j++)
			kernel[i][j]=weights[count++];
	*width=w;
	*height=h;
	return(kernel);
}

double *CNN_getOutputKernel(struct CNN *CNN_v)
{
	return( Neuron_getWeights(&CNN_v->m_N2[0]) );
}
