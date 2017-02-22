
#ifndef _TreeCANN_h
#define _TreeCANN_h


#include "mex.h"
#include <stdio.h>
#include <vector>
#include <limits.h>
#include <string.h>
#include <stdlib.h>


typedef struct BITMAP {
  int w, h;
  unsigned char **line;
  unsigned char *data;
} BITMAP;


/**********************************************/
BITMAP *create_bitmap_patch(int w, int h);
BITMAP *create_bitmap(int w, int h, int d = 4);

/**********************************************/

void clear(BITMAP *bmp);
void clear_to_color(BITMAP *bmp, int c);
void destroy_bitmap(BITMAP *bmp);

/**********************************************/

#define XY_TO_INT(x, y) (((y)<<12)|(x))
#define INT_TO_X(v) ((v)&((1<<12)-1))
#define INT_TO_Y(v) ((v)>>12)

BITMAP *convert_knn_field(const mxArray *A, int add_const = 0);
BITMAP *convert_field(const mxArray *A);
BITMAP *convert_field(const mxArray *X, const mxArray *Y);
void reset_field(BITMAP * field, int set_const = 0);
BITMAP *convert_bitmap_pad(const mxArray *A, int pad_size = 0 , unsigned int pad_val = 0);
/************************************************************/

BITMAP *convert_bitmap(const mxArray *A);
BITMAP *convert_bitmapf(const mxArray *A);
mxArray *bitmap_to_array(BITMAP *a);

#endif
