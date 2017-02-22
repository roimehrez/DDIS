%
% Use ia_process to emboss an image.
%

clear all

addpath flann-1.6.11-src/src/matlab

imA = im2double(imread('examples/newflower_A.jpg'));
imAp = im2double(imread('examples/newflower_emboss.jpg'));
imB = im2double(imread('examples/newshore.jpg'));

options.scaleFeatures = true;

imBp = ia_process(rgb2gray(imA), imAp(:,:,3), rgb2gray(imB), options);

figure(1); clf;
subplot(221); imshow(imA); title('A')
subplot(222); imshow(imAp); title('A''')
subplot(223); imshow(imB); title('B')
subplot(224); imshow(imBp); title('B''');
