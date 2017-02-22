function [DIS, rectDIS] = computeDIS(I,T,patchSize, approximated)
% inputs:   
%   I = target image (RGB)
%   T = template (RGB)
%   patchSize = size of the patch [3]
%   approximated - toggles the usage of ANN (TreeCANN).
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

fastDiversity = 1;

if ~exist('approximated','var')
    approximated = 1;
end

sT = size(T);
%% first step - NN fiels
nnfApprox = zeros(size(I,1),size(I,2));
nnfExact = zeros(size(I,1),size(I,2));
% aproximated params
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
if approximated %using TreeCANN
    [~, nnf_X , nnf_Y] = run_TreeCANN(I,T,patchSize,S_grid,T_grid,train_patches,num_PCA_dims,eps,knn,S_win,T_win,second_phase);
    nnf_X1 = nnf_X(1:end-patchSize+1,1:end-patchSize+1);
    nnf_Y1 = nnf_Y(1:end-patchSize+1,1:end-patchSize+1);
    %remove patchSize from end
    nnfApprox(1:end-patchSize+1,1:end-patchSize+1) = sub2ind(sT(1:2),nnf_Y1,nnf_X1);
    nnf=nnfApprox;
else
    nnf_XYD = ENN_matching(I, T, patchSize);
    nnf_X2 = nnf_XYD(1:end-patchSize+1,1:end-patchSize+1,1)+1;
    nnf_Y2 = nnf_XYD(1:end-patchSize+1,1:end-patchSize+1,2)+1;
    %remove patchSize from end
    nnfExact(1:end-patchSize+1,1:end-patchSize+1) = sub2ind(sT(1:2),nnf_Y2,nnf_X2);
    nnf=nnfExact;
end

%% second step unique
%padding
[m_nnf,n_nnf] = size(nnf);
nnf_padd = repmat(feval(class(nnf), 0), size(nnf)+sT(1:2)-1);
nnf_padd(floor((sT(1)-1)/2)+(1:m_nnf),floor((sT(2)-1)/2)+(1:n_nnf)) = nnf;

if fastDiversity
    DIS = diversity('DiversityCount',int32(nnf_padd),int32(sT(1)),int32(sT(2)));%mex version
else
    DIS = diversityScan_matlab(nnf,double(sT(1:2)));%matlab version
end

%% find target 
rectDIS = findTargetLocation(DIS,'',[sT(2) sT(1)]);
end



function dis = diversityScan_matlab(nnf, sz)

% Expand nnf
[mnnf,nnnf] = size(nnf);
nnf_padd = mkconstarray2(class(nnf), 0, size(nnf)+sz-1);
nnf_padd(floor((sz(1)-1)/2)+(1:mnnf),floor((sz(2)-1)/2)+(1:nnnf)) = nnf;

% Create dis
rows = 0:(sz(1)-1);
cols = 0:(sz(2)-1);
dis = zeros(size(nnf));

% Apply fun to each neighborhood of a
for i=1:mnnf
    for j=1:nnnf
        x = nnf_padd(i+rows,j+cols);
        dis(i,j) = numel(unique(x));
    end
end

end
