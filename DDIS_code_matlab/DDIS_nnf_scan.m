
function [DDIS, rectDDIS] = DDIS_nnf_scan(nnf, sT, h, fastDiversity)
%------------------------------------------------------------------------%
% Copyright 2017 Itamar Talmi and Roey Mechrez

% nnf - NN field W X H X 2 (x ind and y ind for each patch)
% sT - size of the template
% h - bandwidth parameter (usually 1)
% fastDiversity - flag using c++ code [true]
%
%DDIS is the likelihood map
%------------------------------------------------------------------------%

if ~exist('fastDiversity','var')
    fastDiversity = 1;
end

if fastDiversity
    [ySrc,xSrc] = ind2sub(sT, (0:(sT(1)*sT(2)))'); %starting from 0 since its the value of the padding which apears in the nnf map.
    xyPositions = [xSrc(:), ySrc(:)]';
    xyPositions = xyPositions -1; % updating to cpp indexing

    DDIS = diversity('DeformableDiversity', int32(nnf),int32(sT(1)),int32(sT(2)), int32(xyPositions), h);%mex version
else
    DDIS = DDIS_nnf_scan_matlab(nnf, double(sT(1:2)),h);%matlab version
end

%% find target
padMap = padding( ones(size(DDIS)) , sT(1:2) );
DDIS = padding(DDIS,sT(1:2));

windowSizeDividor = 3;
locSearchStyle = '';
rectDDIS  = findTargetLocation(DDIS,locSearchStyle,[sT(2) sT(1)], windowSizeDividor, true, padMap);

end

%% Matlab impl:
function [DDIS,DIS] = DDIS_nnf_scan_matlab(nnf,sT,h)

% Expand nnf
[mnnf,nnnf] = size(nnf);
mnnf = mnnf - sT(1) + 1;
nnnf = nnnf - sT(2) + 1;

% Create dis
rows = 0:(sT(1)-1);
cols = 0:(sT(2)-1);
DIS = zeros(mnnf,nnnf);
DDIS = zeros(mnnf,nnnf);

% Apply fun to each neighborhood of a
for j=1:nnnf
    for i=1:mnnf
        nnfw = nnf(i+rows,j+cols);                
        [DDIS(i,j)] = ComputeDDISForWindow(nnfw,sT,h);
        DIS(i,j) = numel(unique(nnfw));
    end
end
end

function [DDIS]= ComputeDDISForWindow(nnfw,sT,h)
    [yDest,xDest] = ind2sub(sT, nnfw(:));%-patchSize+1
    [ySrc,xSrc] = ind2sub(sT, (1:(sT(1)*sT(2)))');%-patchSize+1
    u = xDest-xSrc;
    v = yDest-ySrc;
    [~,r] = cart2pol(u,v);
    
    [uniqueIndices, chosenIndexesInNnfw,indTrasform] = unique(nnfw);          %provides sorted unique list of elements
    if numel(uniqueIndices) == 1
        useCountForUniqueIdx = numel(nnfw);
    else
        useCountForUniqueIdx = hist(nnfw(:) ,uniqueIndices)';    %provides a count of each element's occurrence
    end
    diversityPerPatch = useCountForUniqueIdx(indTrasform);
    DIw = exp((1-diversityPerPatch) / h);
    DDIS = sum( DIw' ./ (r'+1) );

end

