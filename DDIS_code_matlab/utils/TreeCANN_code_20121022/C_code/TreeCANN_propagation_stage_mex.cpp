
#include "TreeCANN.h"

#define MAX_WIN_S 32            


unsigned int *convert_line_array(const mxArray *A, int add_const = 0){

	int aw = mxGetDimensions(A)[1];
	unsigned int *data = (unsigned int *) mxGetData(A);

	unsigned int *a = new unsigned int[aw];
	for (int x = 0; x < aw; x++){
		a[x] = data[x]+add_const;
	}
	
	return a;
}

/*******************************************************************************/
/* mexFUNCTION - gateway routine for use with MATLAB                           */
/* Calculates Approximate Nearest Neighbor dense matching between two images   */
/*******************************************************************************/
void mexFunction(int nout, mxArray *pout[], int nin, const mxArray *pin[]) {
	if (nin < 10) { mexErrMsgTxt("Propagation stage called with < 10 input arguments"); }
	
	int num_of_colors = 3;
	const mxArray *A = pin[0];	
	const mxArray *B = pin[1];

	if ( !mxIsUint8(A) || !mxIsUint8(B) ){ 
		mexErrMsgTxt("Mex error: Input image A or B are not uint8!");
	}
	if (mxGetNumberOfDimensions(A) != 3 || mxGetNumberOfDimensions(B) != 3){ 
		mexErrMsgTxt("Mex error: Input image A or B contain less than 3 dims!"); 
	}
	if (mxGetDimensions(A)[2] != 3 || mxGetDimensions(A)[2] != 3){ 
		mexErrMsgTxt("Mex error: Input image A or B are not 3 color images!");
	}

	int patch_w = int(mxGetScalar(pin[2]));
	int a_grid  = int(mxGetScalar(pin[3]));
    int a_win   = int(mxGetScalar(pin[4]));   //must be odd
	int b_win   = int(mxGetScalar(pin[5]));   //must be odd
	const mxArray *A_PATCH_POS_X = pin[6];
	const mxArray *A_PATCH_POS_Y = pin[7];

	const mxArray *A_PATCH_KNN_MAP_X = pin[8];
	const mxArray *A_PATCH_KNN_MAP_Y = pin[9];
	int coh_opt  = int(mxGetScalar(pin[10]));


    int a_w_h = (a_win-1)/2; // half size of the A window
    int b_w_h = (b_win-1)/2; // half size of the B window
	int ah = mxGetDimensions(A)[0]+2*a_w_h;
	int aw = mxGetDimensions(A)[1]+2*a_w_h;
	int bh = mxGetDimensions(B)[0]+2*b_w_h;
	int bw = mxGetDimensions(B)[1]+2*b_w_h;

	int number_of_patches = mxGetDimensions(A_PATCH_POS_X)[1];
	int knn = mxGetDimensions(A_PATCH_KNN_MAP_X)[0];

	BITMAP *a_pad = convert_bitmap_pad(A,a_w_h);
	BITMAP *b_pad = convert_bitmap_pad(B,b_w_h,8);

	BITMAP *nnf_XY_pad = create_bitmap(aw, ah);
	BITMAP *nnf_dist_pad = create_bitmap(aw, ah);
	reset_field(nnf_dist_pad, 10000000);
	
	// '1' should be substructed from the fields because of the different indexing in C (starting from '0' and not from '1')
	unsigned int *A_patch_pos_X = convert_line_array(A_PATCH_POS_X,a_w_h-1);
	unsigned int *A_patch_pos_Y = convert_line_array(A_PATCH_POS_Y,a_w_h-1);
	BITMAP *A_patch_knn_map_X = convert_knn_field(A_PATCH_KNN_MAP_X,b_w_h-1);
	BITMAP *A_patch_knn_map_Y = convert_knn_field(A_PATCH_KNN_MAP_Y,b_w_h-1);


	unsigned int int_win_pad[MAX_WIN_S][MAX_WIN_S];
	for (int dy = 0; dy < MAX_WIN_S; ++dy){
		for (int dx = 0; dx < MAX_WIN_S; ++dx){
			int_win_pad[dy][dx] = 0;
		}
	}

	BITMAP *win_pad = create_bitmap(patch_w+a_win,patch_w+a_win);
	clear(win_pad);
	destroy_bitmap(win_pad);
	
	//Compare best 'k-NN' in the original image space and find min distance
	for (int n = 0; n < number_of_patches; n++){
		int x = A_patch_pos_X[n];
		int y = A_patch_pos_Y[n];
		
		unsigned int *nnf_XY_pad_map   = &((unsigned int *) nnf_XY_pad->line[y])[x];
		unsigned int *nnf_dist_pad_map = &((unsigned int *) nnf_dist_pad->line[y])[x];

		int min_patch_dist = 10000000;
		for (int k = 0; k < knn; ++k){
			int x_map = ((unsigned int *)A_patch_knn_map_X->line[n])[k];
			int y_map = ((unsigned int *)A_patch_knn_map_Y->line[n])[k];

			int patch_dist = 0;
			for (int dy = 0; dy < patch_w; ++dy){

				int *a_row_color = &((int *) a_pad->line[y+dy])[x];
				int *b_row_color = &((int *) b_pad->line[y_map+dy])[x_map];
				
				for (int dx = 0; dx < patch_w; ++dx){
					int a_color = a_row_color[dx];
					int b_color = b_row_color[dx];

					int diff_r = (a_color&255)-(b_color&255);
					int diff_g = ((a_color>>8)&255)-((b_color>>8)&255);
					int diff_b = (a_color>>16)-(b_color>>16);

					patch_dist += diff_r*diff_r + diff_g*diff_g + diff_b*diff_b;
				}
				if(patch_dist > min_patch_dist){ break; } //early termination

			}
			if(patch_dist < min_patch_dist){
				min_patch_dist = patch_dist;
				*nnf_XY_pad_map = XY_TO_INT(x_map, y_map);
				*nnf_dist_pad_map = patch_dist;
			}

		}
	}


	if (coh_opt){
		for (int n = 0; n < number_of_patches; n++){ 

			int x = A_patch_pos_X[n];
			int y = A_patch_pos_Y[n];
			
			int xy_map = ((int *) nnf_XY_pad->line[y])[x];
			int x_map = INT_TO_X(xy_map);
			int y_map = INT_TO_Y(xy_map);
			
			for (int j = -(b_win-a_win)/2; j <=(b_win-a_win)/2 ; j++){
				for (int i = -(b_win-a_win)/2; i <= (b_win-a_win)/2; i++){
					
					// calc Integral Image on SoD between two image blocks
					for (int dy = 0; dy < patch_w+a_win-1; ++dy){
						int *a_row_color = &((int *) a_pad->line[y-a_w_h+dy])[x-a_w_h];
						int *b_row_color = &((int *) b_pad->line[y_map-a_w_h+dy+j])[x_map-a_w_h+i];
						
						unsigned int *int_win_pad_row_prev = &(int_win_pad[0+dy][1]);
						unsigned int *int_win_pad_row = &(int_win_pad[1+dy][1]);
						
						int prev_row_sum=0;
						for (int dx = 0; dx < patch_w+a_win-1; ++dx){
							
							int a_color = a_row_color[dx];
							int b_color = b_row_color[dx];
							
							int diff_r = (a_color&255)-(b_color&255);
							int diff_g = ((a_color>>8)&255)-((b_color>>8)&255);
							int diff_b = (a_color>>16)-(b_color>>16);
							
							prev_row_sum = prev_row_sum + diff_r*diff_r + diff_g*diff_g + diff_b*diff_b;
							int_win_pad_row[dx] = prev_row_sum+int_win_pad_row_prev[dx];
						}
					}
					
					// calc patch distance and find minimum
					unsigned int new_dist = 0;
					for (int dy = -a_w_h; dy <= a_w_h; ++dy){
						unsigned int *win_pad_row_first = &(int_win_pad[a_w_h+dy][a_w_h]);
						unsigned int *win_pad_row_last  = &(int_win_pad[patch_w+a_w_h+dy][a_w_h]);
						
						unsigned int *nnf_dist_pad_row = &((unsigned int *) nnf_dist_pad->line[y+dy])[x];
						unsigned int *nnf_XY_pad_row = &((unsigned int *) nnf_XY_pad->line[y+dy])[x];
						
						for (int dx = -a_w_h; dx <= a_w_h; ++dx){
							new_dist = win_pad_row_first[dx] - win_pad_row_first[dx+patch_w]\
								-win_pad_row_last[dx]  + win_pad_row_last[dx+patch_w];
							
							if(new_dist<nnf_dist_pad_row[dx]){
								nnf_dist_pad_row[dx] = new_dist;
								nnf_XY_pad_row[dx] = XY_TO_INT(x_map+dx+i, y_map+dy+j);
							}
						}

					}


				}
			}

		}//for (int n = 0; n < number_of_patches; n++)
	} //if(coh_opt)


	/* copy result to the output array */
	mwSize dims[2] = { ah, aw };
	mxArray *NNF_X_PAD_new = mxCreateNumericArray(2, dims, mxINT32_CLASS, mxREAL);
	mxArray *NNF_Y_PAD_new = mxCreateNumericArray(2, dims, mxINT32_CLASS, mxREAL);
	mxArray *NNF_DIST_PAD_new = mxCreateNumericArray(2, dims, mxINT32_CLASS, mxREAL);

	int *data_X = (int *) mxGetData(NNF_X_PAD_new);
	int *data_Y = (int *) mxGetData(NNF_Y_PAD_new);
	int *data_DIST = (int *) mxGetData(NNF_DIST_PAD_new);

	for (int y = 0; y < ah; y++) {

		int *nnf_XY_pad_row = (int *) nnf_XY_pad->line[y];
		int *nnf_dist_pad_row = (int *) nnf_dist_pad->line[y];

		for (int x = 0; x < aw; x++) {
			int xy_pointer = nnf_XY_pad_row[x];
			data_X[y+x*ah] = INT_TO_X(xy_pointer);
			data_Y[y+x*ah] = INT_TO_Y(xy_pointer);
			data_DIST[y+x*ah] = nnf_dist_pad_row[x];
		}
	}

	pout[0] = NNF_X_PAD_new;
	pout[1] = NNF_Y_PAD_new;
	pout[2] = NNF_DIST_PAD_new;

	/* release memory */
	destroy_bitmap(a_pad);
	destroy_bitmap(b_pad);
	destroy_bitmap(nnf_XY_pad);
	destroy_bitmap(nnf_dist_pad);

	delete A_patch_pos_X;
	delete A_patch_pos_Y;
	destroy_bitmap(A_patch_knn_map_X);
	destroy_bitmap(A_patch_knn_map_Y);

}

