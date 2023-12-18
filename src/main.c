#include <string.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#include "LpiImage.h"

extern int callbackStillImage(struct lpiImage* image, int *X, int *Y, int *H, int *W, float *O);
extern int callbackFrameImage(struct lpiImage *image, int *X, int *Y, int *H, int *W, float *O);
extern void InitCFF();

extern double my_gettime(void);

#define MAX_IMAGES	256
#define MAX_FACES	64

int main(int argc, char *argv[])
{
	FILE *fp;
	char buf[512], name[512];
	int count = 0;
	struct lpiImage *src[MAX_IMAGES] = {0};
	double t1, t2, t3;
	int i, j;

	InitCFF();
	fp=fopen("list.txt", "r");


	t1 = my_gettime();
	while( fgets(buf, sizeof(buf), fp)!=NULL )
	{
		strcpy(name,"");
		sscanf(buf, "%s", name);
//		printf("%s\n", name);
		src[count]=lpiImage_loadPGM(name);
		count++;
		if (count == MAX_IMAGES) {
			printf("too many images...\n");
			exit(1);
		}
	}

	t2 = my_gettime();

	for (i = 0; i < count; i++) {
		int X[MAX_FACES], Y[MAX_FACES], H[MAX_FACES], W[MAX_FACES];
		float O[MAX_FACES];

		int nofFaces=callbackStillImage(src[i], X, Y, H, W, O);
    for(j=0;j<nofFaces;j++)
		{
			printf("FACE[%d]: %d\t%d\t%d\t%d\t%f\n", j, X[j], Y[j], H[j], W[j], O[j]);
		}

		printf("%d : Faces = %d\n", i, nofFaces);
    // lpiImage_savePGM(src[i], "xxx.pgm");
		lpiImage_lpiReleaseImage(&src[i]);

	}
	t3 = my_gettime();

	printf("TIME FOR LOADING IMAGES	= %f\n", t2-t1);
	printf("TIME FOR FACE DETECTION	= %f\n", t3-t2);
	printf("TOTAL EXECUTION TIME	= %f\n", t3-t1);

	exit(0);
}
