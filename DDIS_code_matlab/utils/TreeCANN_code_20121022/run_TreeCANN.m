function [nnf_dist nnf_X nnf_Y runtime] = run_TreeCANN (A, B, patch_w, ...
    A_grid, B_grid, num_of_train_patches, num_PCA_dims, eps, num_of_ann_matches, A_win, B_win, second_phase)
% TreeCANN - Kd-Tree Coherence Nearest Neighbor algorithm
%
% usage: .... = TreeCANN(A,B, [patch_w=8], [A_grid = 2], [B_grid = 2], [num_of_train_patches = 100], [num_PCA_dims=patch_w/2+3], [eps=3], [num_of_ann_matches=4], [A_win=2*A_grid+1], B_win,[second_phase=1])
%
% This function runs the TreeCANN ( Kd-Tree Coherence Nearest Neighbor) algorithm
% to compute the approximate dense nearest neighbor field (correspondence) between images A and B
% -----------------------------------------------------------------------------------------------------------------
%
% Inputs:
% - - - - - - -
% 1] A - an uint8 RGB image, the source of the NN field.
% 2] B - an uint8 RGB image, the tatrget of the NN field.
% 3] patch_w  -  the dimension [wigth in pixels] of a  patch.
% 4] A_grid - sparse grid parmeter for image A
% 5] B_grid - sparse grid parmeter for image B 
% 6] First phase parameters (Kd-tree)
%     6.1] num_of_train_patches - for PCA vectors calculation
%     6.2] num_PCA_dims - patches are reduced to
%     6.3] eps - kd-tree approximation parameter
%     6.4] num_of_ann_matches - returned after the kd-tree search
% 7] Seconnd phase parameters (Propagation)
%     7.1] A_win
%     7.2] B_win
%     7.3] second_phase - when = '0' propagation stage is disabled
%
% Outputs:
% - - - - - - - -
% 1] nnf_dist - distance field A->B
% 2] nnf_X  - X map 
% 3] nnf_Y  - Y map
% 4] runtime(i)
%    4.1] i=1 - Patch extraction
%    4.2] i=2 - PCA vector calculation
%    4.3] i=3 - Dimensionality reduction
%    4.4] i=4 - Kd-tree construction
%    4.5] i=5 - Kd-tree search
%    4.6] i=6 - Propagation stage

runtime = [];

if (nargin < 2) ;   error('Too few inputs'); end
if (~isa(A,'uint8'))  || ( ~isa(B,'uint8'));     error('One of the input images is not Uint8'); end

if (~exist('patch_w','var'));   patch_w = 8; end
if (~exist('A_grid','var'));     A_grid = 2   ; end
if (~exist('B_grid','var'));     B_grid = A_grid; end

% init  First phase parameters (Kd-tree)
if (~exist('num_of_train_patches','var')); num_of_train_patches = 100;  end
if (~exist('num_PCA_dims','var')) ;                     num_PCA_dims = 3 + patch_w/2; end
if (~exist('num_of_ann_matches','var'));       num_of_ann_matches = 4;           end
if (~exist('eps','var'));   eps = 3; end

% init  Second phase parameters (Propagation)
if (~exist('second_phase','var'));  second_phase = 1; end

if (~exist('A_win','var'));    A_win = 2*A_grid+1;
elseif mod(A_win,2) == 0
        error('A_win parameter must be odd');
end

if (~exist('B_win','var'));   B_win = A_win+2;
elseif mod(B_win,2) == 0
        error('B_win parameter must be odd');
end

[A_patch_pos_X  A_patch_pos_Y] = extract_patch_position(size(A), patch_w, A_grid);
[B_patch_pos_X  B_patch_pos_Y] = extract_patch_position(size(B), patch_w, B_grid);

%---------------Patch extraction------------------------------------------------%

tic;
%%%%%%%%%%%%
runtime(1) = toc; %disp(['patch construction time: ', num2str(toc), ' sec']);

%----------------PCA vector calculation-------------------------------------%
tic;

A_patch_learn = extract_patches_for_pca(A, patch_w, A_patch_pos_X, A_patch_pos_Y, num_of_train_patches/2);
B_patch_learn = extract_patches_for_pca(B, patch_w, B_patch_pos_X, B_patch_pos_Y, num_of_train_patches/2);

train_patches = [A_patch_learn B_patch_learn] ;

%train_patches = train_patches';
[M,N] = size(train_patches);

% subtract off the mean for each dimension
mn = mean(train_patches,2);
train_patches = single(train_patches)-repmat(mn,1,N);

% construct the matrix Y
Y = train_patches' / sqrt(N-1);

[PC, SCORE, variances]  = princomp(Y);

%project the original data
PC_reduction = PC(:,1:num_PCA_dims);
runtime(2)  = toc; %disp(['PCA construction time: ', num2str(toc), ' sec']);

%----------------Dimensionality reduction--------------------------------%
tic;
[A_patch B_patch] = TreeCANN_reduce_patches(A, B, patch_w, A_grid, B_grid, PC_reduction);
runtime(3)  = toc; %disp(['PCA reduction time: ', num2str(toc), ' sec']);

%----------------kd-tree construction-----------------------------------------%
tic;
B_patch_ann = ann(B_patch);
runtime(4) = toc; %disp(['Kd-tree construction time: ', num2str(toc), ' sec']);

%----------------kd-tree search-----------------------------------------------------%
tic;
[ANN_index min_dist] = ksearch(B_patch_ann, A_patch, num_of_ann_matches, eps);
A_patch_index = sub2ind([size(A,1) size(A,2)],A_patch_pos_Y,A_patch_pos_X);
ANN_index(double(ANN_index)>numel(B_patch_pos_X)) = 1;
A_patch_knn_map_X = B_patch_pos_X(ANN_index);
A_patch_knn_map_Y = B_patch_pos_Y(ANN_index);
B_patch_ann = close(B_patch_ann);
runtime(5) = toc; %disp(['Kd-tree search time: ', num2str(toc), ' sec']);

%----------------Propagation stage ----------------------------------------------%
tic;
[nnf_X_pad nnf_Y_pad nnf_dist_pad ] = TreeCANN_propagation_stage(uint8(A), uint8(B), patch_w, A_grid, A_win, B_win , uint32(A_patch_pos_X) , uint32(A_patch_pos_Y) , uint32(A_patch_knn_map_X) , uint32(A_patch_knn_map_Y) ,second_phase);
nnf_X_pad = nnf_X_pad+1;
nnf_Y_pad = nnf_Y_pad+1;
runtime(6) = toc; %disp(['Propagation time: ', num2str(toc), ' sec']);

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
A_w_h = (A_win-1)/2; % half size of the A_win
B_w_h = (B_win-1)/2; % half size of the B_win
nnf_X = nnf_X_pad(1+A_w_h:end-A_w_h , 1+A_w_h:end-A_w_h) - B_w_h;
nnf_Y = nnf_Y_pad(1+A_w_h:end-A_w_h , 1+A_w_h:end-A_w_h) - B_w_h;
nnf_dist = nnf_dist_pad(1+A_w_h:end-A_w_h , 1+A_w_h:end-A_w_h);

end


function [patches]  = extract_patches_for_pca(im, patch_w, pos_X, pos_Y, num_of_train_patches)

rand_indx = randi(size(pos_X,2),num_of_train_patches,1);
pos_X = pos_X(rand_indx);
pos_Y = pos_Y(rand_indx);

patch_size = patch_w*patch_w*size(im,3);
patches = zeros(patch_size, num_of_train_patches);
for i=1:length(rand_indx)
    patch  = im(  pos_Y(i):pos_Y(i)+patch_w -1, pos_X(i):pos_X(i)+patch_w-1 ,:) ; 
    %patches(:,i) = reshape(patch, patch_size, 1);
     patches(:,i) = reshape(permute(patch, [2 1 3] ),[prod(numel(patch)) 1] );
end

end


function [im_patch_pos_X  im_patch_pos_Y] = extract_patch_position(im_size,  patch_w, im_grid_step )

x = 1:im_grid_step:im_size(2) - patch_w+1;
y = 1:im_grid_step:im_size(1) - patch_w+1;
[X Y] = meshgrid(y,x);

im_patch_pos_X = reshape(Y,1,numel(Y));
im_patch_pos_Y = reshape(X,1,numel(X));

end
