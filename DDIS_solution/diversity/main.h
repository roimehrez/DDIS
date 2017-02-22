#pragma once

#include "mex.h"

// Expose two available interfaces:
//	diversity('DiversityCount', nnfMat, windowM, windowN, differentWeightStartIndex[optional], differentWeight[optional]);
//  diversity('weightedDiversityCount', nnfMat, windowM, windowN, weights); 
//  diversity('DeformableDiversity', nnfMat, windowM, windowN, xyPositions, [optional]h); 
//  note that M represents rows and N represents columns
void mexFunction(int outputElementCount, mxArray *outputs[], int inputsElementCount, const mxArray *inputs[]);
