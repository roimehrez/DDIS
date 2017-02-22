%------------------------------------------------------------------------%
% Copyright 2017 Itamar Talmi and Roey Mechrez
% For noncommercial use only.
%
% Please cite the appropriate paper(s) if used in research:
%
% Template Matching with Deformable Diversity Similarity
% Talmi, Itamar and Mechrez, Roey and Zelnik-Manor, Lihi
% arXiv preprint arXiv:1612.02190, 2016
% https://arxiv.org/abs/1612.02190
%
% Main contact: 
% - itamar.talmi@gmail.com  (Itamar)
% - roimehrez@gmail.com (Roey)
% Version: 1.0, 2017-02-21
%------------------------------------------------------------------------%
clear all;
%% add paths
addpath(genpath('..\DDIS_bin'));
addpath(genpath('utils'));
run(fullfile(getenv('MATCONV18'),'matlab', 'vl_setupnn.m')) ;

%% set params
% clear all;
dframe = 25;
databaseFolder = 'originalCVPR';%sprintf('dfrm%d', dframe); %'originalCVPR';
dataDir = '.\ExampleImage\';%fullfile('..', 'BBS datasets', databaseFolder);

pz = 3; % non-overlapping patch size used for pixel descirption

rng(791);
colors = distinguishable_colors(23);
colors(1:4 ,:)=[];
colors([3 5 7 8] ,:)=[];

%% load images and target location
pairNum = 1;
swap = false;
displayGT = true;
[I,Iref,T,rectT,rectGT] = utils.loadImageAndTemplate(pairNum,dataDir, swap);%number-case number [I,Iref,T,rectT,gtRect]

% can dynamically load Template and Target image using:
% [I,Iref,T,rectT,rectGT] = utils.loadImageAndTemplateDynamic( [image1], [image2] );

fprintf('T size %dX%d\n', size(T,1),size(T,2));
fprintf('Target image size %dX%d\n', size(I,1),size(I,2));

%% adjust image and template size so they are divisible by the patch size 'pz'
szI = size(I);
disArr = {};
Rects = {};
Names = {};
ind=1;
%% run

%-------------------------------------------------------------
tic;
[heatmap, rectDDIS]   = computeDDIS(I, T, pz); %core function
runtime(ind)=toc;
disArr{ind}=heatmap;
Rects{ind}=rectDDIS;
Names{ind} = 'DDIS RGB';
ind=ind+1;

%-------------------------------------------------------------
if ~exist('net','var')
    [ net, gpuN ] = loadNet();    % loading imagenet-vgg-verydeep-19.mat
end
tic;
[heatmap, rectDDIS]   = computeDDIS_deep(I,T, net, gpuN, 'L2'); %core function
runtime(ind)=toc;
disArr{ind}=heatmap;
Rects{ind}=rectDDIS;
Names{ind} = 'DDIS deep L2';
ind=ind+1;

%-------------------------------------------------------------
% if ~exist('net','var')
%     [ net, gpuN ] = loadNet();    % loading imagenet-vgg-verydeep-19.mat
% end
% tic;
% [heatmap, rectDDIS]   = computeDDIS_deep(I,T, net, gpuN, 'dotP'); %core function
% runtime(ind) = toc;
% disArr{ind}=heatmap;
% Rects{ind}=rectDDIS;
% Names{ind} = 'DDIS deep dotP';
% ind=ind+1;

%-------------------------------------------------------------
% tic;
% [heatmap, rectDDIS]   = computeDIS(I,T, pz); %core function
% runtime(ind)=toc;
% disArr{ind}=heatmap;
% Rects{ind}=rectDDIS;
% Names{ind} = 'DIS RGB';
% ind=ind+1;
%-------------------------------------------------------------

total = length(Rects);
colors = colors(1:total,:);

%% compute overlap with ground-truth
Overlaps = {};
for i=1:length(Names)
    Overlaps{i} = rectOverlap(rectCorners(rectGT), rectCorners(Rects{i}));
end

%% plot results
f=  figure;

rectWidth = 5;
subtightplot(1,2,1);imshow(Iref);hold on;
r = rectangle('position',rectT,'linewidth',rectWidth,'edgecolor',[0 1 0]);
plot(nan,nan,'s','markeredgecolor',get(r,'edgecolor'),'markerfacecolor',get(r,'edgecolor'),'linewidth',3);hold off;
subtightplot(1,2,2);imshow(I);hold on;
for j=1:total
    rct(j) = rectangle('position',Rects{j},'linewidth',rectWidth,'edgecolor',colors(j,:));
    plt(j) = plot(nan,nan,'s','markeredgecolor',get(rct(j),'edgecolor'),'markerfacecolor',get(rct(j),'edgecolor'),'linewidth',3);
    NamesWithOL{j} = [Names{j}, sprintf(' Ol=%.2f',Overlaps{j})];
end

j = total+1;
if displayGT
    rct(j) = rectangle('position',rectGT,'linewidth',rectWidth,'edgecolor',[0,1,0]);
    plt(j) = plot(nan,nan,'s','markeredgecolor',get(rct(j),'edgecolor'),'markerfacecolor',get(rct(j),'edgecolor'),'linewidth',3);
    Names{j}='GroundTruth';
    NamesWithOL{j}='GroundTruth';
end
legend(plt,NamesWithOL,'location','southeast');set(gca,'fontsize',12);

f2=  figure;
f2.Name = f.Name;
for j=1:total
    subtightplot(2,ceil(total/2),j);imagesc(disArr{j});
    rectangle('position',Rects{j},'linewidth',rectWidth,'edgecolor',colors(j,:));colormap jet;axis image;axis off;colorbar;title(NamesWithOL{j});
end

clear mex


