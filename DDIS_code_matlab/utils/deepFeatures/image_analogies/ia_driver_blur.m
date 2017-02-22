
%
% Use ia_process to blur an image.
%
% Currently this converts the images into HSV space and just blurs the
% value channel and leaving the hue and saturation channels unchanged.
% This was done to avoid the cost of processing each color channel
% separately (which is what should probably be done).
%
clear all

addpath flann-1.6.11-src/src/matlab

imA = rgb2hsv(im2double(imread('examples/newflower_A.jpg')));
imAp = rgb2hsv(im2double(imread('examples/newflower_blur.jpg')));
imB = rgb2hsv(im2double(imread('examples/newshore.jpg')));

imBp = ia_process(imA(:,:,3), imAp(:,:,3), imB(:,:,3));

% add the H and S channels to the 
imBp = cat(3, imB(:,:,[1 2]), imBp);

imA = hsv2rgb(imA);
imAp = hsv2rgb(imAp);
imB = hsv2rgb(imB);
imBp = hsv2rgb(imBp);

figure(1); clf;
subplot(221); imshow(imA); title('A')
subplot(222); imshow(imAp); title('A''')
subplot(223); imshow(imB); title('B')
subplot(224); imshow(imBp); title('B''');
