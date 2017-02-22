close all;
%clear all;

%addpath('D:\studies\second degree\thesis\code\PM with PCA\C++ code\PM_local_optimization\PM_local_optimization');

addpath('./matlab_tools/ann_wrapper');
addpath('./C_code');

im_size = 0.1; %MP
patch_w = 8;

A_org = imread('Avatar_FULL_1080p_06004.jpg');
B_org = imread('Avatar_FULL_1080p_06026.jpg');
B_org = imread('7.jpg');
scale_factor = sqrt(im_size*10^6/(size(A_org,1)*size(A_org,2)));
A = imresize(A_org, scale_factor,'bilinear');
B = imresize(B_org, scale_factor,'bilinear');
A = lab2uint8(rgb2lab(A));
B = lab2uint8(rgb2lab(B));

%figure; imshow(A);
%figure; imshow(B);

nnf_X = zeros(size(A(:,:,1)));       
nnf_Y = zeros(size(A(:,:,1)));

%%
disp(['test 1 ---- Calculating TreeCANN with default parameters (grid = 2) ----']);
[nnf_dist, nnf_X , nnf_Y, runtime] = run_TreeCANN(uint8(A),uint8(B),patch_w);

temp_nnf = sqrt(single(nnf_dist(1:end-patch_w,1:end-patch_w)));
TreeCANN_dist = sum(sum(temp_nnf))/numel(temp_nnf);
disp(['grid =2; TreeCANN time: ', num2str(sum(runtime)), ' sec, TreeCANN avrg. dist : ',num2str(TreeCANN_dist)  char(13)]);

ann = cat(3,nnf_X,nnf_Y);
% reconstruction
c_style_CSH_ann = AnnFromMatlab2c(cat(3,ann,zeros(size(A(:,:,1)))));

% the PatchMatch reconstruction function
CSH_A_fromB = votemex(B, c_style_CSH_ann, [], 'cpu', patch_w);

A2 = lab2rgb(lab2double(CSH_A_fromB));
imshow2(A2,lab2rgb(lab2double(A)));




%%
S_grid = 3;
T_grid = 3;

disp(['test 2 ---- Calculating TreeCANN with runtime-mind parameters (grid = 3) ----']);

[nnf_dist, nnf_X , nnf_Y, runtime] =run_TreeCANN(uint8(A),uint8(B),patch_w,S_grid,T_grid);

temp_nnf = sqrt(single(nnf_dist(1:end-patch_w,1:end-patch_w)));
TreeCANN_dist = sum(sum(temp_nnf))/numel(temp_nnf);
disp(['grid =3; TreeCANN time: ', num2str(sum(runtime)), ' sec, TreeCANN avrg. dist : ',num2str(TreeCANN_dist)  char(13)]);

%%
S_grid = 1;
T_grid = 1;

disp(['test 3 ---- Calculating TreeCANN with accuracy-mind parameters (grid = 1) ----']);

[nnf_dist, nnf_X , nnf_Y, runtime] =run_TreeCANN (uint8(A),uint8(B),patch_w,S_grid,T_grid);

temp_nnf = sqrt(single(nnf_dist(1:end-patch_w,1:end-patch_w)));
TreeCANN_dist = sum(sum(temp_nnf))/numel(temp_nnf);
disp(['grid =1; TreeCANN time: ', num2str(sum(runtime)), ' sec, TreeCANN avrg. dist : ',num2str(TreeCANN_dist)  char(13)]);

%%
S_grid = 1;
T_grid = 1;
S_win = 3;    %must be odd
T_win = 5;    %must be odd

eps = 2;
num_PCA_dims = 9;
train_patches = 100;
knn = 5;
second_phase = 1;

disp(['test 4 ---- Calculating TreeCANN with even more accuracy-mind parameters (grid = 1) ----']);

[nnf_dist, nnf_X , nnf_Y, runtime] =run_TreeCANN (uint8(A),uint8(B),patch_w,S_grid,T_grid,train_patches,num_PCA_dims,eps,knn,S_win,T_win,second_phase);

temp_nnf = sqrt(single(nnf_dist(1:end-patch_w,1:end-patch_w)));
TreeCANN_dist = sum(sum(temp_nnf))/numel(temp_nnf);
disp(['grid =1; TreeCANN time: ', num2str(sum(runtime)), ' sec, TreeCANN avrg. dist : ',num2str(TreeCANN_dist)  char(13)]);

%%
disp(['Calculating Exact NN matching for reference to Ground Truth... This may take a while ( about 1 min. for 0.1MP images )...']);
tic;
nnf = ENN_matching(uint8(A), uint8(B), patch_w); 
GT_time = toc;
temp_nnf = sqrt(single(nnf(1:end-patch_w,1:end-patch_w,3)));
GT_error = sum(sum(temp_nnf))/numel(temp_nnf);
disp(['GT_time: ', num2str(toc), ' sec  ', 'GT_error: ', num2str(GT_error)]);

