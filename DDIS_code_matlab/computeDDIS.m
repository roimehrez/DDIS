function [ DDIS, rectDDIS ] = computeDDIS(I,T,patchSize, approximated, fastDiversity, conversion)
% inputs:   
%   Iorig = target image (RGB)
%   Torig = template (RGB)
%   patchSize [3]
%   approximated - flags the usage of TreeCANN ANN algorithm [true]
%   fastDiversity = flags the usage of mex file or matlab implemantation [true]
%   conversion = a converstion function for I,T in order to change its representation, 
%   for example, convert ot HSV. [@(I) I]
%
% outputs:  
%   DIS - Diversity Similarity heat map
%   rectDIS - best match rectangle according to the heat map.
%------------------------------------------------------------------------%
% Copyright 2017 Itamar Talmi and Roey Mechrez

% nnf - NN field W X H X 2 (x ind and y ind for each patch)
% sT - size of the template
% h - bandwidth parameter (usually 1)
% fastDiversity - flag using c++ code [true]
%------------------------------------------------------------------------%


if ~exist('approximated','var')
    approximated = 1;
end
if ~exist('fastDiversity','var')
    fastDiversity = 1;
end
if ~exist('conversion','var')
    conversion = @(I) I;
end

h = 1;  % bendwidth parameter

I = im2double(I);
T = im2double(T);
I = conversion(I);
T = conversion(T);
    
sT = size(T);
%% first step - NN fiels
nnfApprox = zeros(size(I,1),size(I,2));
nnf_dist = zeros(size(I,1),size(I,2));
nnfExact = zeros(size(I,1),size(I,2));
% aproximated params for TreeCANN
S_grid = 1;
T_grid = 1;
S_win = 3;    %must be odd
T_win = 5;    %must be odd
eps = 2;
num_PCA_dims = 9;
train_patches = 100;
knn = 5;
second_phase = 1;

I = im2uint8(I);
T = im2uint8(T);
% tic
if approximated %using TreeCANN
    [nnf_dist_temp, nnf_X , nnf_Y] = run_TreeCANN(I,T,patchSize,S_grid,T_grid,train_patches,num_PCA_dims,eps,knn,S_win,T_win,second_phase);
    nnf_X1 = nnf_X(1:end-patchSize+1,1:end-patchSize+1);
    nnf_Y1 = nnf_Y(1:end-patchSize+1,1:end-patchSize+1);
    %remove patchSize from end
    nnf_dist(1:end-patchSize+1,1:end-patchSize+1) = nnf_dist_temp(1:end-patchSize+1,1:end-patchSize+1);
    nnfApprox(1:end-patchSize+1,1:end-patchSize+1) = sub2ind(sT(1:2),nnf_Y1,nnf_X1);
    nnf=nnfApprox;
else
    nnf_XYD = ENN_matching(I, T, patchSize);
    nnf_X2 = nnf_XYD(1:end-patchSize+1,1:end-patchSize+1,1)+1;
    nnf_Y2 = nnf_XYD(1:end-patchSize+1,1:end-patchSize+1,2)+1;
    nnf_dist_temp = nnf_XYD(1:end-patchSize+1,1:end-patchSize+1,3)+1;
    %remove patchSize from end
    nnf_dist(1:end-patchSize+1,1:end-patchSize+1) = nnf_dist_temp(1:end-patchSize+1,1:end-patchSize+1);
    nnfExact(1:end-patchSize+1,1:end-patchSize+1) = sub2ind(sT(1:2),nnf_Y2,nnf_X2);
    nnf=nnfExact;
end
% toc

%% second step DDIS scan the NNF
[DDIS, rectDDIS] = DDIS_nnf_scan(nnf, sT, h, fastDiversity);
end
