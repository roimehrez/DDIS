% This is the main script for run DDIS over dataset
% to download the BBS dataset and add it to BBS_DATA folder
% http://people.csail.mit.edu/talidekel/Code/BBS_code_and_data_release_v1.0.zip
%------------------------------------------------------------------------%
% Copyright 2017 Itamar Talmi and Roey Mechrez
%
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
%% set folders
% ----------------NEED TO UPDATE-----------------
databaseFolder = 'BBS_DATA'; 
resDir = fullfile('..', sprintf('TM_%s_Results', databaseFolder));
dataDir = fullfile('..', 'BBS_DATA', databaseFolder);
% ----------------NEED TO UPDATE-----------------

%% add paths
addpath(genpath('..\DDIS_bin'));
run(fullfile(getenv('MATCONV18'),'matlab', 'vl_setupnn.m')) ;
addpath(genpath('utils'));
addpath(genpath('..\TreeCANN_code_20121022'));
warning('off','MATLAB:colon:nonIntegerIndex');
warning('off','MATLAB:dispatcher:nameConflict');

%% set and initial params
if ~exist(resDir,'dir'),mkdir(resDir);end
pz = 3; % non-overlapping patch size used for pixel descirption
rng(15);
verbose = 1;
startPairIndex = 1;


files = dir(dataDir);
nPairs = (numel(files)-2)/4;
endPairIndex = nPairs+startPairIndex-1;

Overlaps = cell(1,nPairs);
disArr = {};
Rects = {};
Names = {};
runtime = [];
runtimeAll =[];

% load vgg network for deep features extraction.
if ~exist('net','var'), 
    [ net, gpuN ] = loadNet();
end
%% find template for all image-pairs in dataset 
for imInd = startPairIndex:endPairIndex
    tic;
    %% load images and target location
    [I,Iref,T,rectRef,rectGT] = utils.loadImageAndTemplate(imInd, dataDir);%number-case number
    
    %% adjust image and template size so they are divisible by the patch size 'pz'
    szT = size(T);
    szI = size(I);
    
    ind=1;
    h=1;
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
    tic;
    [heatmap, rectDDIS]   = computeDDIS_deep(I,T, net, gpuN, 'L2'); %core function
    runtime(ind)=toc;
    disArr{ind}=heatmap;
    Rects{ind}=rectDDIS;
    Names{ind} = 'DDIS deep L2';
    ind=ind+1;

    %-------------------------------------------------------------
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

    totalExperiments = length(Rects);
    runtimeAll=[runtimeAll; runtime];
    %-------------------------------------------------------------
    % compute overlap with ground-truth
    t = toc;
    fprintf('Pair %03d: finished in %.2f sec (|I| = %dx%d , |T| = %dx%d) \n',imInd,t,szI(1:2),szT(1:2));
    
    %% calculating overlaps
    for j=1:totalExperiments
       Overlaps{j}(imInd) = rectOverlap(rectCorners(rectGT),rectCorners(Rects{j}));
    end
    
    %% ploting and saving
    if ~exist('colors','var')
        colors = distinguishable_colors(23);
        % [~,indx]=ismember([0 1 0],colors,'rows');
        colors(1:4 ,:)=[];
        colors([3 5 7 8] ,:)=[];
        for i = 1:totalExperiments
            Overlaps{i}=nan(nPairs,1);
        end
    end
    rectWidth = 5;
    
    if verbose
        %%%%%%%%%%%%%%%%%%%% saving summary figure %%%%%%%%%%%%%%%%%%%%%%
        h = figure(1);clf;
        h.Units = 'normalized';
        h.Position = [0 0 1 1];
        subtightplot(2,2,1);imshow(Iref) ;
        rectangle('position',rectRef,'linewidth',rectWidth,'edgecolor',[0,1,0]);colorbar;title(sprintf('Pair %03d\nReference image',imInd));
        subtightplot(2,2,2);imshow((I));hold on;
        NamesWithOL = cell(1, length(Names)+1);
        for j=1:totalExperiments
            NamesWithOL{j} = [Names{j}, sprintf(' Ol=%.2f',Overlaps{j}(imInd))];
            rct(j) = rectangle('position',Rects{j},'linewidth',rectWidth,'edgecolor',colors(j,:));
            plt(j) = plot(nan,nan,'s','markeredgecolor',get(rct(j),'edgecolor'),'markerfacecolor',get(rct(j),'edgecolor'),'linewidth',3);
        end
        j = totalExperiments+1;
        rct(j) = rectangle('position',rectGT,'linewidth',rectWidth,'edgecolor',[0,1,0]);
        plt(j) = plot(nan,nan,'s','markeredgecolor',get(rct(j),'edgecolor'),'markerfacecolor',get(rct(j),'edgecolor'),'linewidth',3);
        NamesWithOL{j}='GroundTruth';
                
        colorbar;title({'Query image'});
        
        indToDraw=1;
        subtightplot(2,2,3);imagesc(disArr{indToDraw}); axis image;
        rectangle('position',Rects{indToDraw},'linewidth',rectWidth,'edgecolor',colors(indToDraw,:));
        colormap jet;axis image;colorbar;title(Names{indToDraw});
        
        indToDraw=indToDraw+1;
        %         indToDraw=3;
        if length(disArr)>=2
            subtightplot(2,2,4);imagesc(disArr{indToDraw}); axis image;
            rectangle('position',Rects{indToDraw},'linewidth',rectWidth,'edgecolor',colors(indToDraw,:));
            colormap jet;axis image;colorbar;title(Names{indToDraw});
        end
        
        legend(plt,NamesWithOL,'location','southeast', 'Interpreter', 'none');
        set(gca,'fontsize',12);
        
        F = getframe(h);
        filename = fullfile(resDir,[num2str(imInd),'.jpg']);
        imwrite(F.cdata,filename);
        close(h);
        
        
        %%%%%%%%%%%%%%%%%%%%%% saving seperate images %%%%%%%%%%%%%%%%%%%%
        currentResFolder = fullfile(resDir,'seperated',num2str(imInd));
        if ~exist(currentResFolder,'dir'), mkdir(currentResFolder), end
        
        % save Template
        header = 'T';
        Iwrite = insertShape(Iref,'rectangle',rectRef,'LineWidth',rectWidth,'color','green');
        filename = fullfile(currentResFolder ,sprintf('%s.jpg',header));
        imwrite(Iwrite,filename);
        
        % save target image with results
        header = 'Target';
        NamesWithOL = cell(1, length(Names)+1);
        j = totalExperiments+1;
        Iwrite = insertShape(I,'rectangle',rectGT,'LineWidth',rectWidth,'color','green');
        NamesWithOL{j}='GroundTruth';
        for j=1:totalExperiments
            Overlaps{j}(imInd) = rectOverlap(rectCorners(rectGT),rectCorners(Rects{j}));
            NamesWithOL{j} = [Names{j}, sprintf(' Ol=%.2f',Overlaps{j}(imInd))];
            Iwrite = insertShape(Iwrite,'rectangle',Rects{j},'LineWidth',rectWidth,'color',colors(j,:));
        end
%         legend(plt,NamesWithOL,'location','southeast', 'Interpreter', 'none');set(gca,'fontsize',12);
        filename = fullfile(currentResFolder ,sprintf('%s.jpg',header));
        imwrite(Iwrite,filename);
        
        % save all heat maps
        for indToDraw=1:totalExperiments
            header = sprintf('%s_map', Names{indToDraw});
            Iwrite = ind2rgb(im2uint8(disArr{indToDraw}/max(max(disArr{1}))), jet(255));
            Iwrite = insertShape(Iwrite,'rectangle',Rects{indToDraw},'LineWidth',rectWidth,'color',colors(indToDraw,:));
            filename = fullfile(currentResFolder ,sprintf('%s.jpg',header));
            imwrite(Iwrite,filename);
        end
    end
end

%% build success curve
f = figure(2);clf;
leg={};

    ROC = struct();
    thROC = 0:0.05:1;
    
    % loading data from Arxiv database
    NamesUnion = {};
    OverlapsUnion = {};
       
    toDrawIndx = 1:length(NamesUnion);
    
    %     name =
    for i=1:length(toDrawIndx)
        ind = toDrawIndx(i);
        name = NamesUnion{ind};
        curve = zeros(size(thROC));
        for th = 1:length(thROC)
            curve(th) = sum(OverlapsUnion{ind}>thROC(th))/nPairs;
        end
        ROC.(name) = curve;
    end
    
    methods = fields(ROC);


for j=1:totalExperiments
    for th = 1:length(thROC)
        OverlapCurves{j}(th) = sum(Overlaps{j}>thROC(th))/nPairs;
    end
    plot(thROC, OverlapCurves{j},'linewidth',rectWidth); hold on;
    leg{j}= sprintf('%s(AUC = %.3f)',Names{j}, mean(OverlapCurves{j}(1:end-1)));
end


%% saving curves
grid on;
xlabel('Threshold');
ylabel('Success rate');
title('Area under the curve');
legend(leg, 'Interpreter', 'none');
savefig(fullfile(resDir,'AUC.fig'))

%% print runtime statistics
for i=1:totalExperiments
    name = Names{i};
    fprintf('%s runtime: mean:%.2f, max:%.2f, min:%.2f, std:%.2f \r\n' ...
        ,name...
        ,mean(runtimeAll(:,i))...
        ,max(runtimeAll(:,i))...
        ,min(runtimeAll(:,i))...
        ,std(runtimeAll(:,i))...
        );
end
