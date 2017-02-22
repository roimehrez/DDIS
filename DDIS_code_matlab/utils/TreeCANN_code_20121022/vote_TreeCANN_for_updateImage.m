function A = vote_TreeCANN_for_updateImage(ann,B,mask,options,WarpedImage,dist)
%CSH function
%c_style_ann = AnnFromMatlab2c(cat(3,ann,zeros(size(ann(:,:,1)))));

% the PatchMatch reconstruction function
A = votemex(B, ann, [], 'cputiled', options.width, uint8(repmat(mask,1,1,3)));


% h = 0.1; %Bandwidth paramter of NN weights kernel
% beta = 1 / h^2;
% Dist = double(squeeze(ann(:,:,3,:)))/65025/3;
% W = Dist;
% W = exp(-0.5*W/prod(options.width)/h^2);
% W = bsxfun(@rdivide, W, sum(W, 3));
% 
% B = lab2rgb(double(B));
% A = ind2ImAvg_mex(im2double(B), int32(squeeze(ann(:,:,1,:))+1), int32(squeeze(ann(:,:,2,:))+1), W,options.width, options.width);
% A = rgb2lab(A);

% PsiD  = 1./sqrt(mean((WarpedImage - A).^2, 3) + 1e-3);
% PsiD = repmat(PsiD,[1,1,3]);
% A = (lambda*PsiD.*WarpedImage +  beta * A)./ (lambda.*PsiD + beta );

