
#include "TreeCANN.h"


void reduce_patches_in_image(const BITMAP *im, int patch_w, int grid, int num_of_patches, int vectors_num, float *vectors_pca, mxArray *IM_PATCHES);

/*******************************************************************************/
/* mexFUNCTION - gateway routine for use with MATLAB                           */
/* Calculates Approximate Nearest Neighbor dense matching between two images   */
/*******************************************************************************/
void mexFunction(int nout, mxArray *pout[], int nin, const mxArray *pin[]) {
	if (nin < 4) { mexErrMsgTxt("Reduce patches called with < 5 input arguments"); }
	
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
	int a_grid    = int(mxGetScalar(pin[3]));
	int b_grid    = int(mxGetScalar(pin[4]));

	int patch_s = patch_w*patch_w;
	int ah = mxGetDimensions(A)[0];
	int aw = mxGetDimensions(A)[1];
	int bh = mxGetDimensions(B)[0];
	int bw = mxGetDimensions(B)[1];

	const mxArray *PCA_VECTORS = pin[5];
	int vectors_len = mxGetDimensions(PCA_VECTORS)[0];
	int vectors_num = mxGetDimensions(PCA_VECTORS)[1];

	float *data_PCA_VECTORS = (float *) mxGetData(PCA_VECTORS);
	float *vectors_pca = (float*)malloc(vectors_len*vectors_num*sizeof(float));
	
	for (int v = 0; v < vectors_num; v++) {        // 7
		for (int d = 0; d < vectors_len; d++) {    // 192
			vectors_pca[v*vectors_len + d] = data_PCA_VECTORS[v*vectors_len + d];
		}
	}
 
	BITMAP *a = convert_bitmap_pad(A,0,0);
	BITMAP *b = convert_bitmap_pad(B,0,0);
	
	int A_num_of_patches = ((int)(1+(ah-patch_w)/a_grid))*((int)(1+(aw-patch_w)/a_grid));
	int B_num_of_patches = ((int)(1+(ah-patch_w)/b_grid))*((int)(1+(aw-patch_w)/b_grid));

	mwSize a_dims[2] = { vectors_num , A_num_of_patches };
	mxArray *A_PATCHES = mxCreateNumericArray(2, a_dims, mxSINGLE_CLASS, mxREAL);
	reduce_patches_in_image(a, patch_w, a_grid, A_num_of_patches, vectors_num, vectors_pca, A_PATCHES);


	mwSize b_dims[2] = { vectors_num , B_num_of_patches };
	mxArray *B_PATCHES = mxCreateNumericArray(2, b_dims, mxSINGLE_CLASS, mxREAL);
	reduce_patches_in_image(b, patch_w, b_grid, B_num_of_patches, vectors_num, vectors_pca, B_PATCHES);

	pout[0] = A_PATCHES;
	pout[1] = B_PATCHES;

	/* release memory */
	destroy_bitmap(a);
	destroy_bitmap(b);
	free(vectors_pca);
}


void reduce_patches_in_image(const BITMAP *im, int patch_w, int grid, int num_of_patches, int vectors_num, float *vectors_pca, mxArray *IM_PATCHES){

	int   few = 8;     //must be 8
	float few_dist[8]; //must be 8


	int patch_s = patch_w*patch_w;
	int vectors_len = 3*patch_s;
	float *data_IM_PATCHES = (float *) mxGetData(IM_PATCHES);
	float *few_patches = (float*)malloc(3*few*patch_s*sizeof(float));
	int im_h = im->h;
	int im_w = im->w;

	int patch_count = 0;
	int *patch_pos = (int*)malloc(2*num_of_patches*sizeof(int));
	for (int y = 0; y < im_h-patch_w+1; y+=grid){
		for (int x = 0; x < im_w-patch_w+1; x+=grid){
			patch_pos[2*patch_count+0]=y;
			patch_pos[2*patch_count+1]=x;
			patch_count++;
		}
	}

	for (int p = 0; p < patch_count; p+=few){
		if(p+few >=patch_count){
			few = patch_count - p;
		}

		for (int f = 0; f < few; f++){
			int y = patch_pos[2*(p+f)+0];
			int x = patch_pos[2*(p+f)+1];

			int dim_count = 0;
			for (int dy = 0; dy < patch_w; ++dy){
				int *im_row = &((int *) im->line[y+dy])[x];

				for (int dx = 0; dx < patch_w; ++dx){
					int im_color = im_row[dx];
					float r = (float)(im_color&255);
					float g = (float)((im_color>>8)&255);
					float b = (float)(im_color>>16);
					
					few_patches[f*vectors_len + 0*patch_s + dim_count] = r;
					few_patches[f*vectors_len + 1*patch_s + dim_count] = g;
					few_patches[f*vectors_len + 2*patch_s + dim_count] = b;

					dim_count++;
				}
			}
		}

		for (int v = 0; v < vectors_num; v++){

			for (int f = 0; f < few; f++){	
				few_dist[f] = 0; 	
			}

			for (int dim = 0; dim < vectors_len; dim++ ){

				float pca_val_0 = vectors_pca[v*vectors_len + dim];

				float val_0 = few_patches[                dim];
				float val_1 = few_patches[1*vectors_len + dim];
				float val_2 = few_patches[2*vectors_len + dim];
				float val_3 = few_patches[3*vectors_len + dim];

				few_dist[0] = few_dist[0] + val_0*pca_val_0;
				few_dist[1] = few_dist[1] + val_1*pca_val_0;
				few_dist[2] = few_dist[2] + val_2*pca_val_0;
				few_dist[3] = few_dist[3] + val_3*pca_val_0;

				float val_4 = few_patches[4*vectors_len + dim];
				float val_5 = few_patches[5*vectors_len + dim];
				float val_6 = few_patches[6*vectors_len + dim];
				float val_7 = few_patches[7*vectors_len + dim];	
				
				few_dist[4] = few_dist[4] + val_4*pca_val_0;
				few_dist[5] = few_dist[5] + val_5*pca_val_0;
				few_dist[6] = few_dist[6] + val_6*pca_val_0;
				few_dist[7] = few_dist[7] + val_7*pca_val_0;
			}

			for (int f = 0; f < few; f++){	
				data_IM_PATCHES[vectors_num*(p+f) + v] = few_dist[f];
			}

		}
	} //end for (int p = 0; p < patch_count; p+=few){ 

	free(patch_pos);
	free(few_patches);

}