#include "FastConvUtils.h"


double **convolveOutput(double ***fms, int nofFMS, double *weights, int width, int height)
{
	int i, j, k;
	double **out=Alloc2DDouble(width, height);

	for(i=0;i<height;i++)
		for(j=0;j<width;j++)
		{
			double product=weights[0];
			for(k=0;k<nofFMS;k++)
				product += fms[k][i][j] * weights[k+1];

			out[i][j]=Neuron_ActivFunc(product);
		}
	return(out);
}


double **subSample(double **input, int width, int height, double coeff, double bias, int x_disp, int y_disp)
{
	int i, j;
	int outWidth = width/2;
	int outHeight = height/2;
	double **out=Alloc2DDouble(outWidth, outHeight);

		//subsampling
	for(i=0;i<height-1-y_disp;i+=2)
		for(j=0;j<width-1-x_disp;j+=2)
		{
			double sum = input[i+y_disp][j+x_disp] + input[i+1+y_disp][j+x_disp]
				+input[i+y_disp][j+1+x_disp] + input[i+1+y_disp][j+1+x_disp];
			sum *= coeff;
			sum += bias;
			out[i/2][j/2]=Neuron_ActivFunc(sum);
		}
	return(out);
}



double **convolve(double **input, int width, int height, double **kernel, double bias, int kernel_dim, int *ow, int *oh)
{
	int i, j, k, l;
	int outWidth = width - kernel_dim + 1;
	int outHeight = height - kernel_dim + 1;
	double **out=Alloc2DDouble(outWidth, outHeight);


	int ws=kernel_dim/2;
	for(i=ws;i<height-ws;i++)
		for(j=ws;j<width-ws;j++)
		{
			double product=bias;
				//for every weight of the conv. kernel
			for(k=-ws;k<=ws;k++)
				for(l=-ws;l<=ws;l++)
					product += kernel[k+ws][l+ws] * input[i+k][j+l];
			out[i-ws][j-ws]=product;
		}
	*ow=outWidth;
	*oh=outHeight;
	return(out);
}



double **convolveNeuron(double **input, int width, int height, double **kernel,
	double bias, int kernel_width, int kernel_height, int *ow, int *oh)
{
	int i, j, k, l;
	int outWidth = width - kernel_width + 1;
	int outHeight = height - kernel_height + 1;
	double **out=Alloc2DDouble(outWidth, outHeight);

	int wsWidth=kernel_width/2;
	int wsHeight=kernel_height/2;

	for(i=wsHeight;i<height-wsHeight;i++)
		for(j=wsWidth;j<=width-wsWidth;j++)
		{
			double product=bias;
				//for every weight of the conv. kernel
			for(k=-wsHeight;k<=wsHeight;k++)
				for(l=-wsWidth;l<wsWidth;l++)
					product += kernel[k+wsHeight][l+wsWidth] * input[i+k][j+l];

			out[i-wsHeight][j-wsWidth]=Neuron_ActivFunc(product);
		}

	*ow=outWidth;
	*oh=outHeight;
	return(out);
}



double **add(double **input1, double **input2, int width, int height)
{
	int i, j;
	double **out=Alloc2DDouble(width, height);

		//subsampling
	for(i=0;i<height;i++)
		for(j=0;j<width;j++)
			out[i][j]=input1[i][j]+input2[i][j];
	return(out);
}
