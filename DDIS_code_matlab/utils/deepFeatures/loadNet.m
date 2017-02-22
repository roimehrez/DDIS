function [ net, gpuN ] = loadNet()

gpuN = gpuDeviceCount;
if gpuN>0 
    % setup GPU
    gpuDevice(1);
end
%load the net
%need to update the path---------------------------------------------
% you can downlaod the net from http://www.vlfeat.org/matconvnet/pretrained/
net = load(fullfile(fileparts(mfilename('fullpath')),'imagenet-vgg-verydeep-19.mat'));
if isfield(net, 'net'), net = net.net; end
net = vl_simplenn_tidy(net);
%remove layers and data which are not in use
net.meta = rmfield( net.meta,'classes');
net.layers = net.layers(1:29);
% move to gpu
if gpuN>0
    net = vl_simplenn_move(net,'gpu');
end
end

