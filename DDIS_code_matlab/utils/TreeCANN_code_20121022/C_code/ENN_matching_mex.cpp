

#include "TreeCANN.h"


// calculate Integral Image on Square of Difference between two images
void calc_II_on_SoD(BITMAP *a, BITMAP *b, int win_h, int win_w, int a_x_l, int a_y_l, int b_x_l, int b_y_l, /*output*/ BITMAP *int_win_pad);
void calc_min_patch_dist(BITMAP *int_win_pad, int patch_w, int win_h, int win_w, int a_x_l, int a_y_l, int b_x_l, int b_y_l, /*output*/ BITMAP *nnf_XY, BITMAP *nnf_dist);


/*******************************************************************************/
/* mexFUNCTION - gateway routine for use with MATLAB                           */
/* Calculates Exact Nearest Neighbor dense matching between two images         */
/*******************************************************************************/
void mexFunction(int nout, mxArray *pout[], int nin, const mxArray *pin[]) {
	
	if (nin < 3) { mexErrMsgTxt("Mex error: nnf_gt called with < 3 input arguments!"); }

	int num_of_colors = 3;
	const mxArray *A = pin[0];	
	const mxArray *B = pin[1];
	int patch_w = int(mxGetScalar(pin[2]));	

	if ( !mxIsUint8(A) || !mxIsUint8(B) ){ 
		mexErrMsgTxt("Mex error: Input image A or B are not uint8!");
	}
	if (mxGetNumberOfDimensions(A) != 3 || mxGetNumberOfDimensions(B) != 3){ 
		mexErrMsgTxt("Mex error: Input image A or B contain less than 3 dims!"); 
	}
	if (mxGetDimensions(A)[2] != 3 || mxGetDimensions(A)[2] != 3){ 
		mexErrMsgTxt("Mex error: Input image A or B are not 3 color images!");
	}
	
	int ah = mxGetDimensions(A)[0];
	int aw = mxGetDimensions(A)[1];
	int bh = mxGetDimensions(B)[0];
	int bw = mxGetDimensions(B)[1];

	BITMAP *a = convert_bitmap(A);
	BITMAP *b = convert_bitmap(B);

	BITMAP *nnf_XY   = create_bitmap(aw, ah);
	BITMAP *nnf_dist = create_bitmap(aw, ah);
	reset_field(nnf_dist, 100000000);

	BITMAP *int_win_pad = create_bitmap(aw+1, ah+1);
	reset_field(int_win_pad, 0);

////////////////////////////////////////////////////////////////////
int a_y_l=0;
int a_y_r=patch_w-1;
int b_y_l=bh-patch_w;
int b_y_r=bh-1;

for (int dy = 0; dy < ah+bh-2*patch_w+1; ++dy){

    int a_x_l = 0;
    int a_x_r = patch_w-1;
    int b_x_l = bw-patch_w;
    int b_x_r = bw-1;
    
	for (int dx = 0; dx < aw+bw-2*patch_w+1; ++dx){

        int win_w = a_x_r - a_x_l+1;
        int win_h = a_y_r - a_y_l+1;
	
		//compute difference and integral image
		calc_II_on_SoD(a, b, win_h, win_w, a_x_l, a_y_l, b_x_l, b_y_l, int_win_pad);
		
		//compute patch distance and find minimum
		calc_min_patch_dist(int_win_pad, patch_w, win_h, win_w, a_x_l, a_y_l, b_x_l, b_y_l, nnf_XY, nnf_dist);


		//order of the if's can't be changed
		if (a_x_r+1 > aw-1){ b_x_r = b_x_r-1; }
		if (b_x_l == 0    ){ a_x_l = a_x_l+1; }
        
		if (a_x_r+1 <= aw-1){ a_x_r = a_x_r+1; }
		if (b_x_l   >     0){ b_x_l = b_x_l-1; }
	}
	
	if (a_y_r+1 > ah-1) { b_y_r = b_y_r-1; }
	if (b_y_l   ==  0 ) { a_y_l = a_y_l+1; }
	if (a_y_r+1 <= ah-1) { a_y_r = a_y_r+1; }
	if (b_y_l   >     0) { b_y_l = b_y_l-1; }

}

  /* copy result to the output array */
  mwSize dims[3] = { ah, aw, 3 };
  mxArray *nnf = mxCreateNumericArray(3, dims, mxINT32_CLASS, mxREAL);
  int *data = (int *) mxGetData(nnf);
  int *xchan = &data[0];
  int *ychan = &data[aw*ah];
  int *dchan = &data[2*aw*ah];
  for (int y = 0; y < ah; y++) {

    int *nnf_XY_row   = (int *) nnf_XY->line[y];
    int *nnf_dist_row = (int *) nnf_dist->line[y];

    for (int x = 0; x < aw; x++) {
      int map = nnf_XY_row[x];
      xchan[y+x*ah] = INT_TO_X(map);
      ychan[y+x*ah] = INT_TO_Y(map);
      dchan[y+x*ah] = nnf_dist_row[x];
    }

  }
  pout[0] = nnf;
  
  /* release memory */
  destroy_bitmap(a);
  destroy_bitmap(b);
  destroy_bitmap(nnf_XY);
  destroy_bitmap(nnf_dist);
  destroy_bitmap(int_win_pad);
	
} // end mexFunction

////////////////////////////////////////////////////////////////////

void calc_II_on_SoD(BITMAP *a, BITMAP *b, int win_h, int win_w, int a_x_l, int a_y_l, int b_x_l, int b_y_l, /*output*/ BITMAP *int_win_pad){
	
	for (int y = 0; y < win_h; ++y){ 
		int *a_row_color = &((int *) a->line[a_y_l+y])[a_x_l];
		int *b_row_color = &((int *) b->line[b_y_l+y])[b_x_l];
		
		unsigned int *int_win_pad_row_prev = &((unsigned int *) int_win_pad->line[0+y])[1];
		unsigned int *int_win_pad_row      = &((unsigned int *) int_win_pad->line[1+y])[1];
		
		int prev_row_sum=0;
		for (int x = 0; x < win_w; ++x){
			int a_color = a_row_color[x];
			int b_color = b_row_color[x];
			
			int diff_r = (a_color&255) - (b_color&255);
			int diff_g = ((a_color>>8)&255) - ((b_color>>8 )&255);
			int diff_b = (a_color>>16) - (b_color>>16);
			
			prev_row_sum = prev_row_sum + diff_r*diff_r + diff_g*diff_g + diff_b*diff_b;
			int_win_pad_row[x] = prev_row_sum+int_win_pad_row_prev[x];
		}
	}

}


void calc_min_patch_dist(BITMAP *int_win_pad, int patch_w, int win_h, int win_w, int a_x_l, int a_y_l, int b_x_l, int b_y_l, /*output*/ BITMAP *nnf_XY, BITMAP *nnf_dist){
	
	unsigned int cur_dist = 0;
	for (int y = 0; y < win_h-patch_w+1; ++y){ 
		unsigned int *win_pad_row_first = &((unsigned int *) int_win_pad->line[        y])[0];
		unsigned int *win_pad_row_last  = &((unsigned int *) int_win_pad->line[patch_w+y])[0];
		
		unsigned int *nnf_dist_row = &((unsigned int *) nnf_dist->line[a_y_l+y])[a_x_l];
		unsigned int *nnf_XY_row   = &((unsigned int *) nnf_XY->line[a_y_l+y])[a_x_l];
		
		for (int x = 0; x < win_w-patch_w+1; ++x){
			cur_dist = win_pad_row_first[x] - win_pad_row_first[x+patch_w]\
				      -win_pad_row_last[x]  + win_pad_row_last[x+patch_w];
			
			if(cur_dist < nnf_dist_row[x]){
				nnf_dist_row[x] = cur_dist;
				nnf_XY_row[x] = XY_TO_INT(b_x_l+x, b_y_l+y);
			}
		}
	}

}