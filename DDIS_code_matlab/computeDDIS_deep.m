function [ DDIS, rectDDIS ] = computeDDIS_deep(Iorig,Torig, net, gpuN, distMeasure, fastDiversity)
% inputs:   
%   Iorig = target image (RGB)
%   Torig = template (RGB)
%   net = network to use when extracting deep feautes
%   gpuN = 
%   distMeasure - 'L2' or 'DOTP'
%   fastDiversity = flags the usage of mex file or matlab implemantation
%
% outputs:  
%   DIS - Diversity Similarity heat map
%   rectDIS - best match rectangle according to the heat map.
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

if ~exist('fastDiversity','var')
    fastDiversity = 1;
end
if ~exist('distMeasure','var')
    distMeasure = 'L2';
end

h = 1;  % bendwidth parameter

I = im2uint8(Iorig);
T = im2uint8(Torig);
I = deepFeatures(net,I,gpuN);
T = deepFeatures(net,T,gpuN);


sT = size(T);
sI = size(I);
Ivec = reshape(I, sI(1)*sI(2), sI(3));
Tvec = reshape(T, sT(1)*sT(2), sT(3));

[Tvec, Ivec] = whitening(Tvec, Ivec);
   
%% first step - NN fiels    
if strcmp(distMeasure, 'L2')
    k=1;
    params.algorithm = 'kdtree';
    params.trees = 8;
    params.checks = 64;
    [nnf, distP] = flann_search(Tvec', Ivec',k,params);
else
    % normalizing:
    Tvec = normalizeVectors(Tvec, 2);
    %Ivec = normalizeVectors(Ivec, 2);

    distP = Ivec*Tvec';
    nnw = reshape(distP,[sI(1:2),sT(1)*sT(2)]);
    [~,nnf] = max(nnw,[],3);
end

nnf = reshape(nnf, sI(1:2));

%% second step DDIS scan the NNF
[DDIS, rectDDIS] = DDIS_nnf_scan(nnf, sT, h, fastDiversity);
end


function normed = normalizeVectors(vecs, dim)
    normesVecs = sqrt(dot(vecs,vecs, dim));
    normed = bsxfun(@rdivide, vecs, normesVecs );
end

function F_out = normelizeRows(F_in, mean_in, std_in)
    F_out = bsxfun(@minus, F_in, mean_in) ;

    if ~exist('std_in','var')
        F_out = bsxfun(@rdivide, F_out, std_in) ;
    end
end

function [Tvec, Ivec] = whitening(Tvec, Ivec)
M = mean(Tvec);
S = std(Tvec);
S(S<0.001)=1;

Ivec = normelizeRows(Ivec, M, S);
Tvec = normelizeRows(Tvec, M, S);
end