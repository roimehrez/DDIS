% imBp = ia_process(imA, imAp, imB, options)
%
% Use Image Analogies (Herzmann et. al.) to generate an image.
%
%
% Original Author: Nathan Jacobs
%
function imBp = ia_process(imA, imAp, imB, options)

if ~exist('options', 'var')
  options = struct;
end

if ~isfield(options, 'scaleFeatures')
  options.scaleFeatures = false;
end

if ~isfield(options, 'sourceWeight')
  options.sourceWeight = .5;
end

% a filter to make the middle more important
h = fspecial('gaussian', [5 5], 2);

% since we are going in column order, this is a mask of where already
% filled in pixels will be in our output image
goodPixels = true(5,5); goodPixels(3,3:5) = false; goodPixels(4:5,:) = false;

szA = size(imA);
szB = size(imB);

% just in case we want to manipulate the features and keep the raw images
% around
fA = imA;
fB = imB;

% scale the features to be in the correct range
if options.scaleFeatures
  fA = fA - mean(fA(:)); fA = fA ./ norm(fA(:));
  fB = fB - mean(fB(:)); fB = fB ./ norm(fB(:));
end

%
% build features from A (using a 5x5 neighborhood)
% stack different channels vertically
%
fAneigh = [];
for ix = 1:size(fA,3)
  fAneigh = ...
    [fAneigh
    im2col(padarray(fA(:,:,ix), [2 2], 'replicate'), [5 5], 'sliding')];
end

% apply gaussian weights to empahsize the middle pixel
fAneigh = bsxfun(@times, repmat(h(:), [size(fA,3), 1]), fAneigh);

%
% build features for A'
%
fApneigh = [];
for ix = 1:size(imAp,3)
  fApneigh = ...
    [fApneigh
    im2col(padarray(imAp(:,:,ix), [2 2], 0), [5 5], 'sliding')];
end

% apply gaussian weights to empahsize the middle pixel (also ignore empty
% pixels in B')
fApneigh = fApneigh(repmat(goodPixels, [size(imAp, 3), 1]),:);
fApneigh = bsxfun(@times, ...
  repmat(h(goodPixels), [size(imAp, 3), 1]), fApneigh);

% weight source and target image equally
scaA = mean(sqrt(sum(fAneigh.^2,1))) \ (options.sourceWeight);
scaAp = mean(sqrt(sum(fApneigh.^2,1))) \ (1-options.sourceWeight);
fAneigh = fAneigh .* scaA;
fApneigh = fApneigh .* scaAp;

%
% build search structure
%

dataset = [fAneigh;fApneigh];

if exist('flann_build_index', 'file')
  [index, params] = flann_build_index(dataset, struct);
else
  disp('The file ''flann_build_index'' was not found.'); 
  disp('if you had this nearest neighbor search would work faster.')
  disp('This means you do not have the FLANN library (fast approximate nearest neighbors).');
  disp('You can get this library from http://www.cs.ubc.ca/~mariusm/index.php/FLANN/FLANN.');
end

imBp = nan(szB);

fBpad = padarray(fB, [2 2], 'replicate');
s = zeros([size(imBp,1), size(imBp,2), 2]);

% start adding pixels
for ixB = 1:szB(1)
  fprintf('Processing row %d of %d\n', ixB, szB(1));
  for jxB = 1:szB(2)
    
    %
    % build a feature that describes my local neighborhood
    %
    
    % find nearest neighbor in upper image
    feat = fBpad(ixB + (0:4), jxB + (0:4),:);
    fBppad = padarray(imBp, [2 2], 0); % lazy padding to avoid edge cases
    featp = fBppad(ixB + (0:4), jxB + (0:4),:);
    featp = featp(repmat(goodPixels, [1 1 size(imAp,3)]));
    
    % apply gaussian weights to empahsize the middle pixel
    feat = bsxfun(@times, feat, h);
    featp = bsxfun(@times, featp, repmat(h(goodPixels), [size(imAp,3) 1]));
    feat = [feat(:).*scaA; featp.*scaAp];

    %
    % search for nearest neighbor of (ixB, jxB) in A, A' using this feature
    %
    
    if exist('index', 'var')
      % find nearest neighbor the fast way
      [result, ~] = flann_search(index,feat,1,params);
    else
      % find nearest neighbor the slow way
      dists = L2_distance(feat, dataset);
      [~, result] = min(dists);
    end
            
    % find location in A'
    [ixAopt jxAopt] = ind2sub(szA, result);

    % add the pixel value to the output
    imBp(ixB, jxB, :) = imAp(ixAopt, jxAopt,:);
    s(ixB, jxB,:) = [ixAopt jxAopt];
    
  end
  
  if mod(ixB, 10) == 0

    figure(1); clf;
    subplot(321); imshow(imA); title('A')
    subplot(322); imshow(imAp); title('A''')
    subplot(323); imshow(imB); title('B')
    subplot(324); imshow(imBp); title('B''');
    subplot(325); imagesc(s(:,:,1), [1 size(imBp,1)]); title('i source')
    subplot(326); imagesc(s(:,:,2), [1 size(imBp,2)]); title('j source')
    
    drawnow; pause(eps);
  end
  
end

if exist('index', 'var')
  flann_free_index(index);
end