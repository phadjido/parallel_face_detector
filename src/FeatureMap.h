// FeatureMap.h: interface for the FeatureMap class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _FEATURE_MAP_H_
#define _FEATURE_MAP_H_

#include <stdio.h>

struct FeatureMap
{
	double m_subBias;
	double m_coeff;
	double m_bias;
	double ** m_weights;
	int m_sizeX;
	int m_sizeY;
	int m_windowSize;
};


int FeatureMap_readState(struct FeatureMap *FeatureMap_v, FILE *fp);

//fast conv
double **FeatureMap_getKernel(struct FeatureMap *FeatureMap_v, int *size, double *bias);
void FeatureMap_getSubSample(struct FeatureMap *FeatureMap_v, double *coeff, double *bias);

#endif
