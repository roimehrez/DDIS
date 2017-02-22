
disp('The analogy in ia_driver_numbers currently does not work well.  Why not?  Can you fix it?');

fprintf('\n\nYou can probably ignore these errors...\n')

imA = im2double(imread('examples/oxbow_A.png'));
imAp = im2double(imread('examples/oxbow_A_filtered.png'));
imB = im2double(imread('examples/oxbow_B.png'));

% shrink the images for speed of debugging
imA = imresize(imA, .5);
imB = imresize(imB, .5);
imAp = imresize(imAp, .5);

imBp = ia_process(imA, imAp, imB);

figure(1); clf;
subplot(221); imshow(imA); title('A')
subplot(222); imshow(imAp); title('A''')
subplot(223); imshow(imB); title('B')
subplot(224); imshow(imBp); title('B''');

