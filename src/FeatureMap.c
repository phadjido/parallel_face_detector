// FeatureMap.cpp: implementation of the FeatureMap class.
//
//////////////////////////////////////////////////////////////////////

#include "FeatureMap.h"
#include "MemUtils.h"

#include <stdlib.h>
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//FeatureMap::FeatureMap()
//{
//
//}

//FeatureMap::~FeatureMap()
//{
//
//}



int FeatureMap_readState(struct FeatureMap *FeatureMap_v, FILE *fp)
{
	int i, j;
	double LR;
	char buf[256];

	char ba[256], bb[256], bc[256];

		//ignore comments
	fgets(buf, sizeof(buf), fp);

	fgets(buf, sizeof(buf), fp);
	sscanf(buf, "%lf", &LR);

	fgets(buf, sizeof(buf), fp);
	sscanf(buf, "%d %d %d", &FeatureMap_v->m_sizeY, &FeatureMap_v->m_sizeX, &FeatureMap_v->m_windowSize);

	FeatureMap_v->m_weights=Alloc2DDouble(FeatureMap_v->m_windowSize, FeatureMap_v->m_windowSize);

	for(i=0;i<FeatureMap_v->m_windowSize;i++)
		for(j=0;j<FeatureMap_v->m_windowSize;j++)
		{
			fgets(buf, sizeof(buf), fp);
			sscanf(buf, "%lf", &(FeatureMap_v->m_weights[i][j]));
		}
	fgets(buf, sizeof(buf), fp);
	sscanf(buf, "%s %s %s\n", ba, bb, bc);

	FeatureMap_v->m_bias=atof(ba);
	FeatureMap_v->m_coeff=atof(bb);
	FeatureMap_v->m_subBias=atof(bc);

	return(1);
}


double **FeatureMap_getKernel(struct FeatureMap *FeatureMap_v, int *size, double *bias)
{
	*size=FeatureMap_v->m_windowSize;
	*bias=FeatureMap_v->m_bias;
	return(FeatureMap_v->m_weights);
}
	
void FeatureMap_getSubSample(struct FeatureMap *FeatureMap_v, double *coeff, double *bias)
{
	*coeff=FeatureMap_v->m_coeff;
	*bias=FeatureMap_v->m_subBias;
}
