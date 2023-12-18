// Neuron.cpp: implementation of the Neuron class.
//
//////////////////////////////////////////////////////////////////////

#include "Neuron.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//Neuron::Neuron()
//{
//
//}

//Neuron::~Neuron()
//{
//
//}

int Neuron_readState(struct Neuron *Neuron_v, FILE *fp)
{
	int i;
	double LR, momentum;

	char buf[256];
		//ignore the initial comment
	fgets(buf, sizeof(buf), fp);

	fgets(buf, sizeof(buf), fp);
	sscanf(buf, "%lf %lf", &LR, &momentum);

	fgets(buf, sizeof(buf), fp);
	sscanf(buf, "%d", &Neuron_v->m_Wlength);

	Neuron_v->m_W=(double *)malloc(sizeof(double)*Neuron_v->m_Wlength);

	for(i=0;i<Neuron_v->m_Wlength;i++)
	{
		fgets(buf, sizeof(buf), fp);
		sscanf(buf, "%lf", &(Neuron_v->m_W[i]));
	}
	return(1);
}

double *Neuron_getWeights(struct Neuron *Neuron_v)
{
	return(Neuron_v->m_W);
}

	//NOTE tanh(x)=(exp(x)-exp(-x))/(exp(x)+exp(-x))
double Neuron_ActivFunc(double input)
{
	double expx=exp( lecunB * input);
	double expmx=exp(- lecunB * input);
	return( lecunA *(expx-expmx)/(expx+expmx) );
}

	//NOTE tanh(x)=(exp(x)-exp(-x))/(exp(x)+exp(-x))
float Neuron_ActivFuncF(float input)
{
	double expx=exp( lecunB * ((double)input) );
	double expmx=exp(- lecunB * ((double)input) );
	return( (float) lecunA *(expx-expmx)/(expx+expmx) );
}
