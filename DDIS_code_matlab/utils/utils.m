classdef utils
   ...
    methods(Static)
        function [T,rect,I] = loadTemplate(dataDir)
            imFile = dir(sprintf('%s\\*frm1*.jpg',dataDir));
            rectFile = dir(sprintf('%s\\*frm1*.txt',dataDir));

            I    = imread(fullfile(dataDir,imFile(1).name));
            rect    = round(load(fullfile(dataDir,rectFile(1).name)));
            T = imcrop(I,rect);
        end

        function [I,Iref,T,rectT,gtRect] = loadImageAndTemplate(pairN,dataDir, swap)
			if ~exist('swap','var')
				swap = 0; 
			end

			imFiles = dir(sprintf('%s\\pair%04d*.jpg',dataDir,pairN));
			rectFile = dir(sprintf('%s\\pair%04d*.txt',dataDir,pairN));

			if swap
				imFiles = swap2(imFiles);
				rectFile = swap2(rectFile);
			end

			Iref    = im2single(imread(fullfile(dataDir,imFiles(1).name)));
			I       = im2single(imread(fullfile(dataDir,imFiles(2).name)));

			if size(I,3) ==1 
			   I = cat(3,I,I,I); 
			   Iref = cat(3,Iref,Iref,Iref);
			end

			rectT    = round(load(fullfile(dataDir,rectFile(1).name)));
			gtRect  = round(load(fullfile(dataDir,rectFile(2).name)));
			T = imcrop(Iref,rectT);
		end

		function A=swap2(A)
			tmp = A(1);
			A(1)=A(2);
			A(2)=tmp;
		end
        
        function [I,Iref,T,rectT,gtRect] = loadImageAndTemplateDynamic(nameT, nameTarget, scaleFactorIref, scaleFactorTargetI)
            if nargin<3
                scaleFactorIref = 1;
            end
            if nargin<4
                scaleFactorTargetI = scaleFactorIref;
            end
            Iref = im2double(imread(nameT)); Iref = imresize(Iref, scaleFactorIref);
            f = figure;imshow(Iref);
            rectT = getrect
            close(f);
            
            I = im2double(imread(nameTarget)); I = imresize(I, scaleFactorTargetI);
            f = figure;imshow(I);
            gtRect = getrect
            close(f);
            
            T = imcrop(Iref,rectT);
        end   
    end
end