function [cur_NNF,B] = run_TreeCANN_for_updateImage(A,B,mask,options)

A = lab2uint8(double(A));
mask = imresize(mask,[size(B,1), size(B,2)]);
% cur_NNF = CSH_nn(A,B,...
%     options.width,...
%     options.iterations,...
%     options.k,...
%     0,...
%     mask);
% A(:,:,1) = A(:,:,1) * 2;
% B(:,:,1) = B(:,:,1) * 2;

cur_NNF  = nnmex(A,B, 'cputiled', options.width, [], [], [], [], [], 12, repmat(uint8(mask),1,1,3));



%[~, nnf_X , nnf_Y, ~] = run_TreeCANN(uint8(A),uint8(B),options.patch_size,maskInd);
%construct output as in ImageMelding sNNMex function
%cur_NNF = cat(3,nnf_X,nnf_Y);

end