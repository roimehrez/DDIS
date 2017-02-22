function [rect, score] = findTargetLocation(map,modeType,windowSizeXY, windowSizeFactor, useCenterOfMass, padMap)

if ~exist('windowSizeFactor','var')
    windowSizeFactor = 5;
end

if ~exist('useCenterOfMass','var')
    useCenterOfMass = false;
end
            
if strcmp(modeType,'max')
    [score,ind] = max(map(:));
    [y,x] = ind2sub(size(map),ind);
    rect = [x-(windowSizeXY(1)/2), y-(windowSizeXY(2)/2), windowSizeXY-1];
else
    maxRow=1;
    maxCol=1;
    maxSum=0;
    
    scannedWSizeXY = ceil(windowSizeXY/windowSizeFactor);
    intMap = integralImage(map);
    for colI = 1:size(map,2)-scannedWSizeXY(1)+1
        for rowI = 1:size(map,1)-scannedWSizeXY(2)+1
            currentSum = ...
                intMap(rowI+scannedWSizeXY(2),colI+scannedWSizeXY(1)) ...
                + intMap(rowI, colI) ...
                - intMap(rowI+scannedWSizeXY(2),colI) ...
                - intMap(rowI,colI+scannedWSizeXY(1));

            if currentSum>maxSum  
               maxSum = currentSum;
               maxRow=rowI+floor(scannedWSizeXY(2)/2);
               maxCol=colI+floor(scannedWSizeXY(1)/2);
            end
        end
    end
    maxAvg = maxSum / rowsMultCols(scannedWSizeXY);
    score = maxAvg;
    rect = [maxCol-(windowSizeXY(1)/2), maxRow-(windowSizeXY(2)/2), windowSizeXY-1];
end
    % fine tunning location by central mass
    if useCenterOfMass
        
        widenedRectDims = round(([rect(3) rect(4)] * 1));
        widenedRect = [rect(1:2)-(widenedRectDims-rect(3:4))/2, widenedRectDims];
        
        croppedPadMap = imcrop(padMap, widenedRect);
        
        croppedMap = imcrop(map, widenedRect);
        croppedMap(isnan(croppedMap))=0;
        
        % avoid calculation center of mass to padd areas. symetric crop only
        % relevent data.
        cropRowsCount = sum(    ~croppedPadMap( :, ceil(size(croppedPadMap,2)/2) )    );
        cropColsCount = sum(    ~croppedPadMap( ceil(size(croppedPadMap,1)/2), :  )     );
        if sum(~croppedPadMap( [1,end], ceil(size(croppedPadMap,2)/2) ))==2
            cropRowsCount = ceil(cropRowsCount/2);
        end
        if sum(~croppedPadMap( ceil(size(croppedPadMap,1)/2), [1,end]  ))==2
            cropColsCount = ceil(cropColsCount/2);
        end
        croppedMap= imcrop(croppedMap, [1+cropColsCount, 1+cropRowsCount, windowSizeXY(1)-2*cropColsCount, windowSizeXY(2)-2*cropRowsCount] );
        
        middleCoordiante = 1+(size(croppedMap)-1)/2;
        correctionVec = centerOfMass(croppedMap) - middleCoordiante;
        correctionVec = fliplr(correctionVec);
    %     correctionVecNorm = 10*norm(correctionVec./windowSizeXY)
        rect(1:2)=rect(1:2)+round(correctionVec);
    end
end


function res = rowsMultCols(mat)
res = mat(1)*mat(2);
end