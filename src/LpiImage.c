#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "LpiImage.h"

#define INLINE_255_A
#define INLINE_255_B


struct lpiImage *lpiImage_lpiCreateImage(int width, int height, int block_size)
{
	struct lpiImage *res=(struct lpiImage *)calloc(1, sizeof(struct lpiImage));
	res->width=width;
	res->height=height;
	res->imageData=(unsigned char *)calloc(width*height, block_size);
	return(res);
}

void lpiImage_lpiReleaseImage(struct lpiImage **image)
{
	free( (*image)->imageData );
	free( (*image) );
}


void lpiImage_lpiResize(struct lpiImage *imgIn, struct lpiImage *imgOut)
{
	int i, n, m;
//	int k, l;

	float colRatio=((float)imgIn->width)/((float)imgOut->width);
	float lineRatio=((float)imgIn->height)/((float)imgOut->height);

#if 1
	int *r1=(int *)calloc(imgIn->width*imgOut->height, sizeof(int));
	int *r2=(int *)calloc(imgOut->width*imgOut->height, sizeof(int));
#else
	int *r1=(int *)memalign(64, imgIn->width*imgOut->height*sizeof(int));
	int *r2=(int *)memalign(64, imgOut->width*imgOut->height*sizeof(int));
#endif

	unsigned char *pin=(unsigned char *)imgIn->imageData;
	unsigned char *pout=(unsigned char *)imgOut->imageData;

	int Inwidth = imgIn->width;
	int Inheight = imgIn->height;
	int Outwidth = imgOut->width;
	int Outheight = imgOut->height;
	

		//filtro stis sthles
	for(m=0;m<Outheight;m++)
	{
		float maf;
		int mai;
		int m_Inwidth = m*Inwidth;
		int *r11;

		maf=((float)m)*lineRatio;
		mai=(int)maf;

		r11 = &r1[m_Inwidth];
		{
			register int k;
			register int k_Inwidth;
			unsigned char *pin1;

			k=mai-1;
			k_Inwidth = k*Inwidth;
			pin1 = &pin[k_Inwidth];
			{
			for (n=0; n<Inwidth;n++) {
	    			if( (k<Inheight) && (k>=0) ) {
					float temp = maf - (float)k;
					if (temp < 0.0) temp = -temp;
					if (temp < 1.0) {
						temp = 1.0 -temp;
						temp = ((float)pin1[n])*temp;
						r11[n] += (int) temp;
					}
				}
	    			else if(k<Outheight) {
#ifndef INLINE_255_A
	      				r1tmp += lpiImage_HA(255, maf-(float)k);
#else
					float temp = maf - (float)k;
					if (temp < 0.0) temp = -temp;
					if (temp < 1.0) {
						temp = 1.0 -temp;
						temp = ((float)255)*temp;
						r11[n] += (int) temp;
					}
#endif
				}
			}
			}
			k=mai;
			k_Inwidth = k*Inwidth;
			pin1 = &pin[k_Inwidth];
			{
			for (n=0; n<Inwidth;n++) {
	    			if( (k<Inheight) && (k>=0) ) {
					float temp = maf - (float)k;
					if (temp < 0.0) temp = -temp;
					if (temp < 1.0) {
						temp = 1.0 -temp;
						temp = ((float)pin1[n])*temp;
						r11[n] += (int) temp;
					}
				}
	    			else if(k<Outheight) {
#ifndef INLINE_255_A

	      				r1tmp += lpiImage_HA(255, maf-(float)k);
#else
					float temp = maf - (float)k;
					if (temp < 0.0) temp = -temp;
					if (temp < 1.0) {
						temp = 1.0 -temp;
						temp = ((float)255)*temp;
						r11[n] += (int) temp;
					}
#endif
				}
			}
			}
			k=mai+1;
			k_Inwidth = k*Inwidth;
			pin1 = &pin[k_Inwidth];
			{
			for (n=0; n<Inwidth;n++) {
	    			if( (k<Inheight) && (k>=0) ) {
					float temp = maf - (float)k;
					if (temp < 0.0) temp = -temp;
					if (temp < 1.0) {
						temp = 1.0 -temp;
						temp = ((float)pin1[n])*temp;
						r11[n] += (int) temp;
					}
				}
	    			else if(k<Outheight) {
#ifndef INLINE_255_A
	      				r1tmp += lpiImage_HA(255, maf-(float)k);
#else
					float temp = maf - (float)k;
					if (temp < 0.0) temp = -temp;
					if (temp < 1.0) {
						temp = 1.0 -temp;
						temp = ((float)255)*temp;
						r11[n] += (int) temp;
					}
#endif
				}
			}
			}
		}
    	}

		//filtro stis grammes
	for(m=0;m<imgOut->height;m++) {
		int m_Inwidth = m*Inwidth;
		int m_Outwidth = m*Outwidth;

		int *r22 = &r2[m_Outwidth];

		for(n=0;n<imgOut->width;n++)
		{
			float nbf;
			int nbi;
			int r2tmp = 0;
			register int l;
			int *r11;
			

			nbf=((float)n)*colRatio;
			nbi=(int)nbf;
			l=nbi-1;
			r11 = &r1[m_Inwidth+nbi];
			{
	  			if( (l<Inwidth) && (l>=0) ) {
					float temp= nbf-(double)l;
					if(temp<0.0)
				    		temp=-temp;
				  	if(temp>=1.0) {
					}
					else {
					  	temp=1.0-temp;
  						temp=((float)r11[-1])*temp;
						r2tmp += (int)temp;
					}
				}
	 			else if(l<Outwidth) {
#ifndef INLINE_255_B
	    				r2tmp += lpiImage_HB(255, nbf-(double)l);
#else
					float temp= nbf-(double)l;
					if(temp<0.0)
				    		temp=-temp;
				  	if(temp<1.0) {
					  	temp=1.0-temp;
  						temp=((float)255)*temp;
						r2tmp += (int)temp;
					}
#endif
				}
			}
			l=nbi;
			{
	  			if( (l<Inwidth) && (l>=0) ) {
					float temp= nbf-(double)l;
					if(temp<0.0)
				    		temp=-temp;
				  	if(temp <1.0) {
					  	temp=1.0-temp;
  						temp=((float)r11[0])*temp;
						r2tmp += (int)temp;
					}
				}
	 			else if(l<Outwidth) {
#ifndef INLINE_255_B
	    				r2tmp += lpiImage_HB(255, nbf-(double)l);
#else
					float temp= nbf-(double)l;
					if(temp<0.0)
				    		temp=-temp;
				  	if(temp<1.0) {
					  	temp=1.0-temp;
  						temp=((float)255)*temp;
						r2tmp += (int)temp;
					}
#endif
				}
			}
			l=nbi+1;
			{
	  			if( (l<Inwidth) && (l>=0) ) {
					float temp= nbf-(double)l;
					if(temp<0.0)
				    		temp=-temp;
				  	if(temp<1.0) {
					  	temp=1.0-temp;
  						temp=((float)r11[1])*temp;
						r2tmp += (int)temp;
					}
				}
	 			else if(l<Outwidth) {
#ifndef INLINE_255_B
	    				r2tmp += lpiImage_HB(255, nbf-(double)l);
#else
					float temp= nbf-(double)l;
					if(temp<0.0)
				    		temp=-temp;
				  	if(temp<1.0) {
					  	temp=1.0-temp;
  						temp=((float)255)*temp;
						r2tmp += (int)temp;
					}
#endif
				}
			}
			r22[n]=r2tmp;
//			r2[m_Outwidth+n]=r2tmp;
		}
      	}

	for(i=0;i<Outwidth*Outheight;i++)
	{
		if(r2[i]<0)
			pout[i]=0;
		else if(r2[i]>255)
			pout[i]=255;
		else
			pout[i]=r2[i];
	}

	free(r1);
	free(r2);
}






	//epistrefei to f1(k, l)*h(ma-k)
int /*inline*/ lpiImage_HA(int f1, float d)
{
  	float temp=d;
  	if(temp<0.0)
    		temp=-temp;
	if(temp>=1.0)
    		return(0);
  	temp=1.0-temp;
  	temp=((float)f1)*temp;
	return((int)temp);
/*
	if(temp<0.0)
		return((unsigned char)0);
	else if(temp>255.0)
		return((unsigned char)255);
	else
	  	return((unsigned char)temp);
*/
}

	//epistrefei to f1(k, l)*h(nb-l)
int /*inline*/ lpiImage_HB(int f2, float d)
{
	float temp=d;
	if(temp<0.0)
    		temp=-temp;
  	if(temp>=1.0)
    		return(0);
  	temp=1.0-temp;
  	temp=((float)f2)*temp;
	return((int)temp);
/*
	if(temp<0.0)
		return((unsigned char)0);
	else if(temp>255.0)
		return((unsigned char)255);
	else
	  	return((unsigned char)temp);
*/
}





int lpiImage_savePGM(struct lpiImage *lpiImage_v, char *filename)
{
	int i, j;
	unsigned char *tmp;
	unsigned char *pix;

	FILE *fp=fopen(filename, "wb");
	if(fp==NULL)
	{
		fprintf(stderr, "Cannot open file [%s] for writting PGM.\n", filename);
		return(0);
	}
		//writting headers
	fprintf(fp, "P5\n%d %d\n255\n", lpiImage_v->width, lpiImage_v->height);

	tmp=(unsigned char *)calloc(lpiImage_v->width, sizeof(unsigned char));
	pix=(unsigned char *)lpiImage_v->imageData;
	for(i=0;i<lpiImage_v->height;i++)
	{
		for(j=0;j<lpiImage_v->width;j++)
			tmp[j]=pix[i*lpiImage_v->width+j];
		fwrite(tmp, sizeof(unsigned char), lpiImage_v->width, fp);
	}
	fflush(fp);
	fclose(fp);
	free(tmp);
	return(1);
}


struct lpiImage *lpiImage_loadPGM(char *filename)
{
	int i, j;
	char buf[1000];
	int width, height;
	struct lpiImage *res;
	unsigned char *tmp;


	FILE *fp=fopen(filename, "rb");
	if(fp==NULL)
	{
		fprintf(stderr, "Cannot open file [%s] for reading PGM.\n", filename);
		return(NULL);
	}
		//getting the code
	fgets(buf, sizeof(buf), fp);
	if(strcmp(buf, "P5\n")!=0)
	{
		fprintf(stderr, "Cannot open file [%s] for reading PGM, unknown code[%s].\n", filename, buf);
		fclose(fp);
		return(NULL);
	}
		//ignore comments
	while( fgets(buf, sizeof(buf), fp) != NULL )
		if(buf[0]!='#')
			break;
	sscanf(buf, "%d%d", &width, &height);
		//ingone the "255"
	fgets(buf, sizeof(buf), fp);

	res=lpiImage_lpiCreateImage(width, height, sizeof(unsigned char));
	tmp=(unsigned char *)calloc(width, sizeof(unsigned char));

	for(i=0;i<height;i++)
	{
		int r=fread(tmp, sizeof(unsigned char), width, fp);
		if(r!=width)
		{
			fprintf(stderr, "Error while reading PGM file [%s]\n", filename);
			return(NULL);
		}
		for(j=0;j<width;j++)
			res->imageData[i*width+j]=tmp[j];
	}
	fclose(fp);
	free(tmp);

	strcpy(res->name, filename);
	return(res);
}

