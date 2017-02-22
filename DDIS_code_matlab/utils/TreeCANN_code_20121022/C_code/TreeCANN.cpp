
#include "TreeCANN.h"

#include <stdlib.h>
#include <stdio.h>

//**********************************************/
BITMAP *create_bitmap(int w, int h, int d) {
  BITMAP *ans = new BITMAP();
  ans->w = w;
  ans->h = h;
  ans->data = new unsigned char[d*w*h];
  ans->line = new unsigned char*[h];
  for (int y = 0; y < h; y++) {
    ans->line[y] = &ans->data[y*d*w];
  }
  return ans;
}

BITMAP *create_bitmap_patch(int w, int h) {
  BITMAP *ans = new BITMAP();
  ans->w = w;
  ans->h = h;
  ans->data = new unsigned char[w*h];
  ans->line = new unsigned char*[h];
  for (int y = 0; y < h; y++) {
    ans->line[y] = &ans->data[y*w];
  }
  return ans;
}

/************************************************************/

void clear(BITMAP *bmp) {
  clear_to_color(bmp, 0);
}

void clear_to_color(BITMAP *bmp, int c) {
  for (int y = 0; y < bmp->h; y++) {
    int *row = (int *) bmp->line[y];
    for (int x = 0; x < bmp->w; x++) {
      row[x] = c;
    }
  }
}

void destroy_bitmap(BITMAP *bmp) {
  if (!bmp) { return; }
  delete[] bmp->line;
  delete[] bmp->data;
  delete bmp;
}

/****************************************************/

BITMAP *convert_knn_field(const mxArray *A, int add_const) {
	if (mxGetNumberOfDimensions(A) > 2) { mexErrMsgTxt(" matrix dims > 2"); }

	int h = mxGetDimensions(A)[0];
	int w = mxGetDimensions(A)[1];

	if (mxIsUint32(A))
	{
		unsigned int *data = (unsigned int *) mxGetData(A);
		BITMAP *ans = create_bitmap(h, w);
		for (int y = 0; y < w; y++) {
			int *row = (int *) ans->line[y];
			for (int x = 0; x < h; x++) {
				unsigned int value = data[y*h+x];
				row[x] = value+add_const;
			}
		}
		return ans;
	} else {
		mexErrMsgTxt("bitmap not mxIsUint32");
	}
}


void reset_field(BITMAP * field, int set_const) {

  for (int y = 0; y < field->h ; y++) {
    int *field_row = (int *) field->line[y];
	for (int x = 0; x < field->w; x++) { 
		field_row[x] = set_const;
	}
  }

}

BITMAP *convert_field(const mxArray *A) {
  if (mxGetNumberOfDimensions(A) != 2) { mexErrMsgTxt("distances matrix dims != 2"); }

  int h = mxGetDimensions(A)[0];
  int w = mxGetDimensions(A)[1];

  if (mxIsUint32(A))
  {
    unsigned int *data = (unsigned int *) mxGetData(A);
    BITMAP *ans = create_bitmap(w, h);
    for (int y = 0; y < h; y++) {
      int *row = (int *) ans->line[y];
      for (int x = 0; x < w; x++) {
	    int dist = data[y+x*h];
        row[x] = dist;
      }
    }
    return ans;
  } else {
    mexErrMsgTxt("field not mxIsUint32");
  }
}

BITMAP *convert_field(const mxArray *X, const mxArray *Y) {

  int ndims = mxGetNumberOfDimensions(X);
  if (ndims != 2) {
    char buf[256];
    if (ndims == 1) {
      sprintf(buf, "field dims != 3 (%d 1d array where nn field expected)", mxGetDimensions(X)[0]);
    }else {
      sprintf(buf, "field dims != 3 (%d dimension array where nn field expected)", ndims);
    }
    mexErrMsgTxt(buf);
  }
  if (!mxIsUint32(X)) { mexErrMsgTxt("field is not Uint32"); }

  int h = mxGetDimensions(X)[0];
  int w = mxGetDimensions(X)[1];

  unsigned int *data_X = (unsigned int *) mxGetData(X);
  unsigned int *data_Y = (unsigned int *) mxGetData(Y);
  BITMAP *ann = create_bitmap(w, h);

  for (int y = 0; y < h; y++) {
    int *ann_row = (int *) ann->line[y];
    for (int x = 0; x < w; x++) {
      int xp = data_X[y+x*h];
      int yp = data_Y[y+x*h];

      ann_row[x] = XY_TO_INT(xp, yp);
    }
  }
  return ann;
}

BITMAP *convert_bitmap_pad(const mxArray *A, int pad_size, unsigned int pad_val) {
  if (mxGetNumberOfDimensions(A) != 3) { mexErrMsgTxt("dims != 3"); }
  if (mxGetDimensions(A)[2] != 3) { mexErrMsgTxt("color channels != 3"); }

  if (mxIsUint8(A)) {
	  int h = mxGetDimensions(A)[0];
	  int w = mxGetDimensions(A)[1];

	  BITMAP *ans = create_bitmap(w+2*pad_size, h+2*pad_size);
	  //int pad_color = pad_val|(pad_val<<8)|(pad_val<<16);
	  int pad_color = pad_val<<24;

	  ///////////////////////////
	  for (int y = 0; y < pad_size; y++) {
		  int *row = (int *) ans->line[y];
		  for (int x = 0; x < w+2*pad_size; ++x) { row[x] = pad_color; }
	  }
	  for (int y = pad_size; y < h+pad_size; y++) {
		  int *row = (int *) ans->line[y];

		  for (int x = 0; x < pad_size; ++x) { row[x] = pad_color;  }
		  for (int x = w+pad_size; x < w+2*pad_size; ++x) { row[x] = pad_color;  }
	  }
	  for (int y = h+pad_size; y < h+2*pad_size; y++) {
		  int *row = (int *) ans->line[y];
		  for (int x = 0; x < w+2*pad_size; ++x) { row[x] = pad_color; }
	  }
	  ///////////////////////////

	  unsigned char *data = (unsigned char *) mxGetData(A);
	  for (int y = 0; y < h; y++) {
		  int *row = (int *) ans->line[y+pad_size];
		  for (int x = 0; x < w; x++) {
			  int r = data[y+x*h];
			  int g = data[(y+x*h)+w*h];
			  int b = data[(y+x*h)+2*w*h];
			  row[x+pad_size] = r|(g<<8)|(b<<16);
		  }
	  }
	  return ans;
  } else {
	  mexErrMsgTxt("bitmap not uint8");
  }
}

/*******************************************************************/

BITMAP *convert_bitmap(const mxArray *A) {
  if (mxGetNumberOfDimensions(A) != 3) { mexErrMsgTxt("dims != 3"); }
  if (mxGetDimensions(A)[2] != 3) { mexErrMsgTxt("color channels != 3"); }

  int h = mxGetDimensions(A)[0];
  int w = mxGetDimensions(A)[1];

  if (mxIsUint8(A)) {
    unsigned char *data = (unsigned char *) mxGetData(A);
    BITMAP *ans = create_bitmap(w, h);
    for (int y = 0; y < h; y++) {
      int *row = (int *) ans->line[y];
      for (int x = 0; x < w; x++) {
	    int r = data[y+x*h];
        int g = data[y+x*h+w*h];
        int b = data[y+x*h+2*w*h];

		row[x] = r|(g<<8)|(b<<16);
      }
    }
    return ans;
  }  else {
	  mexErrMsgTxt("bitmap not uint8");
  }
}

BITMAP *convert_bitmapf(const mxArray *A) {
  if (!(mxGetNumberOfDimensions(A) == 2 ||
       (mxGetNumberOfDimensions(A) == 3 && mxGetDimensions(A)[2] == 3))) { mexErrMsgTxt("float bitmap doesn't have 2 dims or 3 dims with 3 channels"); }

  int h = mxGetDimensions(A)[0];
  int w = mxGetDimensions(A)[1];

  if (mxIsUint8(A)) {
    unsigned char *data = (unsigned char *) mxGetData(A);
    BITMAP *ans = create_bitmap(w, h);
    for (int y = 0; y < h; y++) {
      float *row = (float *) ans->line[y];
      for (int x = 0; x < w; x++) {
        row[x] = data[y+x*h];
      }
    }
    return ans;
  } else if (mxIsDouble(A)) {
    double *data = (double *) mxGetData(A);
    BITMAP *ans = create_bitmap(w, h);
    for (int y = 0; y < h; y++) {
      float *row = (float *) ans->line[y];
      for (int x = 0; x < w; x++) {
        row[x] = data[y+x*h];
      }
    }
    return ans;
  } else {
    mexErrMsgTxt("float bitmap not uint8 or double");
  }
}


mxArray *bitmap_to_array(BITMAP *a) {
  mwSize dims[3] = { a->h, a->w, 3 };
  mxArray *ans = mxCreateNumericArray(3, dims, mxUINT8_CLASS, mxREAL);
  unsigned char *data = (unsigned char *) mxGetData(ans);
  unsigned char *rchan = &data[0];
  unsigned char *gchan = &data[a->w*a->h];
  unsigned char *bchan = &data[2*a->w*a->h];
  for (int y = 0; y < a->h; y++) {
    int *row = (int *) a->line[y];
    for (int x = 0; x < a->w; x++) {
      int c = row[x];
      rchan[y+x*a->h] = c&255;
      gchan[y+x*a->h] = (c>>8)&255;
      bchan[y+x*a->h] = (c>>16);
    }
  }
  return ans;
}