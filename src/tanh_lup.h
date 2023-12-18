#ifndef _TANH_LUP_H_
#define _TANH_LUP_H_


#include<math.h>


#if 1
#define TANH_SIMPLE(x, y) 	if(x>8.0) \
		y=c_la; \
	else if(x<-8.0) \
		y=-c_la; \
else {int tmp_index = ((int)(x*100.0f))+800; y=TANH_LUP[tmp_index];}

#else
#define TANH_SIMPLE(x, y) 	y=c_la*tanhf(c_lb*x);
#endif


void init_tanh();


#endif

