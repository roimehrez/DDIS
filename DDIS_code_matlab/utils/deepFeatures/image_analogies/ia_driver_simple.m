imA = rgb2gray(im2double(imread('examples/newflower_A.jpg')));
imAp = rgb2gray(im2double(imread('examples/newflower_blur.jpg')));
imB = rgb2gray(im2double(imread('examples/newflower_A.jpg')));

imBp = ia_process(imA, imAp, imB);

figure(1); clf;
subplot(221); imshow(imA); title('A')
subplot(222); imshow(imAp); title('A''')
subplot(223); imshow(imB); title('B')
subplot(224); imshow(imBp); title('B''');
