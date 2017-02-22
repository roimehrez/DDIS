% create random dataset and test set
tic
dataset = single(rand(108,200000));
testset = single(rand(108,200000));
% define index and search parameters
params.algorithm = 'kdtree';
params.trees = 8;
params.checks = 64;
% perform the nearest-neighbor search
[result, dists] = flann_search(dataset,testset,5,params);
toc
