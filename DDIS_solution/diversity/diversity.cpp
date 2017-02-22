#include <algorithm>
#include <LIMITS.H>
#include "CountArray.h"
#include "Mat.h"
#include "../mex_opencv_2.4/MxArray.hpp"
#include "../mex_opencv_2.4/EigenExtensions.h"
#include "mexInterface.h"
#include "diversity.h"

using namespace std;
using namespace dis;

void CalcDiversityMap(Mat<int> nnfMat, int windowM, int windowN, vector<float> weights, Mat<float>& diversityScoreMap)
{
	int outputM = diversityScoreMap.height;
	int outputN = diversityScoreMap.width;

	for (int j = 0; j < outputN; ++j)
	{
		WeightedCountArray windowItemsCount(weights);
        
		// first row
		int i = 0;
		if (i < outputM) 
		{
                
			dis::Window<int> w(windowN, windowM, j, i, nnfMat);
			windowItemsCount.CountFrom(w);
			diversityScoreMap(i, j) = std::max(0.0f, windowItemsCount.GetNonZeroElementsWeight());
		}
            
		// all other rows
		for (i = 1; i < outputM; ++i)
		{
			dis::Window<int> removedWindow(windowN, 1, j, i - 1, nnfMat);
			windowItemsCount.ReverseCountFrom(removedWindow);

			dis::Window<int> addedWindow(windowN, 1, j, i + windowM - 1, nnfMat);
			windowItemsCount.CountFrom(addedWindow);
                
			diversityScoreMap(i, j) = std::max(0.0f, windowItemsCount.GetNonZeroElementsWeight());
		}
	}
}

vector<float> BuildWeightVector(Mat<int> nnfMat, int differentWeightStartIndex, int differentWeight)
{
	int nnfArrayLength = nnfMat.width * nnfMat.height;
	const int* nnfArray = &nnfMat(0, 0);
	int maxElement = *std::max_element(nnfArray, nnfArray + nnfArrayLength);
	vector<float> weights(maxElement + 1, 1.0f);
	differentWeightStartIndex = min((int)weights.size(), differentWeightStartIndex);

	for (int j = 0; j < differentWeightStartIndex; ++j)
	{
		weights[j] = 1.0f;
	}

	for (int j = differentWeightStartIndex; j < weights.size(); ++j)
	{
		weights[j] = static_cast<float>(differentWeight);
	}

	return weights;
}

void dis::DiversityCount(mxArray** outputs, int inputsElementCount, const mxArray** inputs)
{
	/*  loading Nearest neighbor field map */
	int windowM, windowN;
	Mat<int> nnfMat = ExtractNnfAndWindow(inputs, windowM, windowN);
	int nnfM = nnfMat.height;
	int nnfN = nnfMat.width;
	int differentWeightStartIndex = INT_MAX;
	int differentWeight = 1;  //same weight

	/*  Arranging output mxArray */
	Mat<float> diversityScoreMap = CreateOutputScoreMap(outputs, windowM, windowN, nnfM, nnfN);

	vector<float> weights = BuildWeightVector(nnfMat, differentWeightStartIndex, differentWeight);
	CalcDiversityMap(nnfMat, windowM, windowN, weights, diversityScoreMap);
}

void dis::WeightedDiversityCount(mxArray** outputs, int inputsElementCount, const mxArray** inputs)
{
	/*  loading Nearest neighbor field map */
	int windowM, windowN;
	Mat<int> nnfMat = ExtractNnfAndWindow(inputs, windowM, windowN);
	int nnfM = nnfMat.height;
	int nnfN = nnfMat.width;
	const int* nnfArray = &nnfMat(0, 0);

	vector<float> weights;
	int maxElement;
	if (inputsElementCount >= Idx::IN_differentWeight)
	{
		int differentWeightStartIndex = INT_MAX;
		if (inputsElementCount > Idx::IN_differentWeightStartIndex) { differentWeightStartIndex = MxArray(inputs[Idx::IN_differentWeightStartIndex]).toInt(); }

		int differentWeight = 1;
		if (inputsElementCount > Idx::IN_differentWeight) { differentWeight = MxArray(inputs[Idx::IN_differentWeight]).toInt(); }
		weights = BuildWeightVector(nnfMat, differentWeightStartIndex, differentWeight);
		maxElement = weights.size() - 1;
	}
	else 
	{
		ASSERT2(inputsElementCount >= Idx::IN_weightsVec + 1, "missing argument: weights vector");
		weights = MxArray(inputs[Idx::IN_weightsVec]).toVector<float>();
		int nnfArrayLength = nnfMat.width * nnfMat.height;
		maxElement = *std::max_element(nnfArray, nnfArray + nnfArrayLength);
	}

	Mat<float> diversityScoreMap = CreateOutputScoreMap(outputs, windowM, windowN, nnfM, nnfN);
	if (weights.size() < maxElement + 1) {
		mexErrMsgTxt("weights vector is too short. Its size is less than the maximum element in the nnf matrix plus one more for zero index (padding)");
	}

	CalcDiversityMap(nnfMat, windowM, windowN, weights, diversityScoreMap);
}
