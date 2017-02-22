%setup MatConvNet
%you need to install MatConvNet, I use version beta18
%and fix the path in the next line-------------------------------------
[ net, gpuN ] = loadNet();

%read an image - keep it in uint8
im = imread('boy.jpg') ;
%extract features
tic;
Fmap = deepFeatures(net,im,gpuN);
toc;
% it takes about 0.1sec
%back to cpu-single
if gpuN>0
    Fmap = gather(Fmap);
end
