#ifndef _LPIIMAGE_H
#define _LPIIMAGE_H


struct lpiImage
{
	char name[512];
	int width;
	int height;
	unsigned char *imageData;
};

struct lpiImage *lpiImage_lpiCreateImage(int width, int height, int pixel_size);
void lpiImage_lpiReleaseImage(struct lpiImage **image);

void lpiImage_lpiResize(struct lpiImage *imgIn, struct lpiImage *imgOut);
int /*inline*/ lpiImage_HA(int f1, float d);
int /*inline*/ lpiImage_HB(int f2, float d);

int lpiImage_savePGM(struct lpiImage *lpiImage_v, char *filename);
struct lpiImage *lpiImage_loadPGM(char *filename);

#endif


