// Neuron.h: interface for the Neuron class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _NEURON_H_
#define _NEURON_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <stdio.h>





//const double lecunA=1.7159;
//const double lecunB=2.0/3.0;

#define lecunA	(1.7159)
#define lecunB	(2.0/3.0)



struct Neuron  
{
	double * m_W;
	int m_Wlength;
};


int Neuron_readState(struct Neuron *Neuron_v, FILE *fp);

double *Neuron_getWeights(struct Neuron *Neuron_v);


/*static*/ double Neuron_ActivFunc(double input);

/*static*/ float Neuron_ActivFuncF(float input);

#endif
