#include <algorithm>
#include "mexInterface.h"
#include "../mex_opencv_2.4/MxArray.hpp"
#include "../mex_opencv_2.4/EigenExtensions.h"
#include "../mex_opencv_2.4/MexAsserts.h"
#include "CountArray.h"
#include "tictoc.h"
#include <thread>
#include "deformableDiversity.h"

using namespace std;
using namespace dis;


int FindMaxNNIndex(const Mat<int>& nnfMat)
{
	int nnfArrayLength = nnfMat.width * nnfMat.height;
	const int* nnfArray = &nnfMat(0, 0);
	int maxElement = *std::max_element(nnfArray, nnfArray + nnfArrayLength);
	return maxElement;
}

double calculateOneWindowScore(Window<int>& win, const Map<MatrixXi>& xyPositions, DeformableDiversityWeightArray windowItemsWeights)
{
	double sum = 0;
	for (auto iter= win.getIterator(); iter.currentItem() != nullptr; iter.next())
	{
		DEBUG_ASSERT(iter.currentItem() != nullptr);
		int nnIndex = *iter.currentItem();
		int i = iter.i();
		int j = iter.j();
		DEBUG_ASSERT(nnIndex == win(i, j));

		auto nnPosition = xyPositions.col(nnIndex);
		auto currentPosition = Vector2i(j, i);
		
		auto diffVector = nnPosition - currentPosition;
		double R = sqrt(diffVector.squaredNorm());
		double weight = windowItemsWeights.GetWeight(nnIndex);
		sum += weight / (R + 1);
	}

	return sum;
}

struct ddisScanParams
{
	int startColIndexInclusive;
	int endColIndexExclusive;
	const Mat<int>* pNnfMat;
	Mat<float>* pScoreMap;
	const Map<MatrixXi>* pXyPositions;
	int windowM;
	int windowN;
	double h;
	int maxElement;

	// default empty Ctor for array constraction
	ddisScanParams(){}

	ddisScanParams(
		int startColIndexInclusive,
		int endColIndexExclusive,
		int windowM, int windowN, int h,
		const Map<MatrixXi>* pXyPositions, Mat<float>* pScoreMap, const Mat<int>* pNnfMat, int maxElement) :
		startColIndexInclusive(startColIndexInclusive), endColIndexExclusive(endColIndexExclusive), 
		pNnfMat(pNnfMat), pScoreMap(pScoreMap), pXyPositions(pXyPositions), 
		windowM(windowM), windowN(windowN), h(h), maxElement(maxElement)
	{
	}
};

void CalcDDISRasterScan(
	Mat<int> nnfMat, int windowM, int windowN, const Map<MatrixXi>& xyPositions, double h,
	Mat<float>& scoreMap_out,
	int startColIndexInclusive, int endColIndexExclusive, int* pMaxElement = nullptr)
{
	int maxElement;
	if (pMaxElement == nullptr)
	{
		maxElement = FindMaxNNIndex(nnfMat);
	}
	else
	{
		maxElement = *pMaxElement;
	}

	ASSERT2(maxElement <= xyPositions.cols(), "the amount of xy positions must be greater or equal to the maximum index in the nnf matrix");

	int rowCount = scoreMap_out.height;
	DeformableDiversityWeightArray windowItemsWeights(1 + maxElement, h);

	for (int j = startColIndexInclusive; j < endColIndexExclusive; ++j)
	{
		// start scanning down using a for loop. first case where i==0 is different:
		int i = 0;
		if (i < rowCount)
		{
			if (j == startColIndexInclusive) {
				// first calculation for top left window
				Window<int> w(windowN, windowM, j, i, nnfMat);
				windowItemsWeights.CountFrom(w);
				scoreMap_out(i, j) = calculateOneWindowScore(w, xyPositions, windowItemsWeights);
			}
			else
			{
				// move right
				Window<int> removedWindow(1, windowM, j - 1, i, nnfMat);
				windowItemsWeights.ReverseCountFrom(removedWindow);

				Window<int> addedWindow(1, windowM, j + windowN - 1, i, nnfMat);
				windowItemsWeights.CountFrom(addedWindow);

				Window<int> w(windowN, windowM, j, i, nnfMat);
				scoreMap_out(i, j) = calculateOneWindowScore(w, xyPositions, windowItemsWeights);
			}
		}

		// scanning down
		for (i = 1; i < rowCount; ++i)
		{
			Window<int> removedWindow(windowN, 1, j, i - 1, nnfMat);
			windowItemsWeights.ReverseCountFrom(removedWindow);

			Window<int> addedWindow(windowN, 1, j, i + windowM - 1, nnfMat);
			windowItemsWeights.CountFrom(addedWindow);

			Window<int> w(windowN, windowM, j, i, nnfMat);
			scoreMap_out(i, j) = calculateOneWindowScore(w, xyPositions, windowItemsWeights);
		}

		// Finished scanning down, moving right and scanning up
		if (++j < endColIndexExclusive)
		{
			// move right, special update case for i==(rowCount-1)
			{
				--i; //correct i back to a valid (and last) index
				Window<int> removedWindow(1, windowM, j - 1, i, nnfMat);
				windowItemsWeights.ReverseCountFrom(removedWindow);

				Window<int> addedWindow(1, windowM, j + windowN - 1, i, nnfMat);
				windowItemsWeights.CountFrom(addedWindow);

				Window<int> w(windowN, windowM, j, i, nnfMat);
				scoreMap_out(i, j) = calculateOneWindowScore(w, xyPositions, windowItemsWeights);
				--i;
			}

			// scanning up
			for (; i >= 0; --i)
			{
				Window<int> removedWindow(windowN, 1, j, i + windowM, nnfMat);
				windowItemsWeights.ReverseCountFrom(removedWindow);

				Window<int> addedWindow(windowN, 1, j, i, nnfMat);
				windowItemsWeights.CountFrom(addedWindow);

				Window<int> w(windowN, windowM, j, i, nnfMat);
				scoreMap_out(i, j) = calculateOneWindowScore(w, xyPositions, windowItemsWeights);
			}
		}
	}
}

void CalcDDISRasterScan(Mat<int> nnfMat, int windowM, int windowN, const Map<MatrixXi>& xyPositions, double h, Mat<float>& scoreMap)
{
	int endColIndexExclusive = scoreMap.width;
	int startColIndexInclusive = 0;
	CalcDDISRasterScan(nnfMat, windowM, windowN, xyPositions, h, scoreMap, startColIndexInclusive, endColIndexExclusive);
}

void CalcDDISRasterScan_ThreadFunc(ddisScanParams p)
{
	CalcDDISRasterScan(*p.pNnfMat, p.windowM, p.windowN, *p.pXyPositions, p.h, *p.pScoreMap, p.startColIndexInclusive, p.endColIndexExclusive, &p.maxElement);
}

Map<MatrixXi> LoadXyPositions(const mxArray** inputs)
{
	MxArray mxXYPositions = MxArray(inputs[Idx::IN_xyPositons]);
	auto xyPositions = mexArray2EigenMat_int(mxXYPositions);

	ASSERT2(
		xyPositions.cols() == 2 || xyPositions.rows() == 2,
		"argument xyPositions must be a matrix of (2 X N) where N is the amount of available patches in the database. N must be greater than the maximum index of in the nnf matrix");

	if (xyPositions.cols() == 2)
	{
		xyPositions.transposeInPlace();
	}

	return xyPositions;
}

void dis::DeformableDiversity(mxArray** outputs, int inputsElementCount, const mxArray** inputs, unsigned nthreads)
{
	if (nthreads <= 0)
	{
		nthreads = NTHREADS;
	}

	/*  loading Nearest neighbor field map */
	int windowM, windowN;
	Mat<int> nnfMat = ExtractNnfAndWindow(inputs, windowM, windowN);
	int nnfM = nnfMat.height;
	int nnfN = nnfMat.width;
	const int* nnfArray = &nnfMat(0, 0);
	ASSERT2(inputsElementCount >= Idx::IN_xyPositons + 1, "missing argument: xy positins matrix");

	Map<MatrixXi> xyPositions = LoadXyPositions(inputs);

	double h = 1;
	if (inputsElementCount >= Idx::IN_h + 1)
	{
		h = MxArray(inputs[Idx::IN_h]).toDouble();
	}

	Mat<float> diversityScoreMap = CreateOutputScoreMap(outputs, windowM, windowN, nnfM, nnfN);

	// Seperate work between threads, building parameters.
	std::vector<ddisScanParams> paramsVec(nthreads);
	std::vector<thread> threadVec(nthreads - 1);  // minus one since using the current thread
	int outputN = diversityScoreMap.width;

	int chunkSize = outputN / nthreads;
	int leftoverModulu = outputN % nthreads;
	int startRangeInclusive = 0;
	int maxElement = FindMaxNNIndex(nnfMat);
	for (int i = 0; i < paramsVec.size(); i++)
	{
		int endRangeExclusive = startRangeInclusive + chunkSize;
		if (leftoverModulu > 0)
		{
			++endRangeExclusive;
			--leftoverModulu;
		}

		paramsVec[i] = ddisScanParams(
			startRangeInclusive, endRangeExclusive, windowM, windowN, h, 
			&xyPositions, &diversityScoreMap, &nnfMat, maxElement);

		startRangeInclusive = endRangeExclusive;
	}

	//Launch threads 1 to NTHREADS-1
	for (int threadInd = 0; threadInd < threadVec.size(); threadInd++)
	{
		threadVec.at(threadInd) = thread(CalcDDISRasterScan_ThreadFunc, std::ref(paramsVec[threadInd]));
	}

	//using the current thread as the last thread, updating a chunk of old patches
	CalcDDISRasterScan_ThreadFunc(paramsVec[nthreads - 1]);

	//Wait for all threads to finish
	for (int threadInd = 0; threadInd < threadVec.size(); ++threadInd)
	{
		threadVec[threadInd].join();
	}
}
