#pragma once
#include <vector>
#include "../mex_opencv_2.4/MexAsserts.h"
#include "Window.h"
#include <map>
#include <unordered_set>

using namespace std;

#define FOREACH_COLUMN_SCAN(j,i,win) for (int j = 0; j < win.width_; ++j) \
									for (int i = 0; i < win.height_; ++i)

#define FOREACH_COLUMN_SCAN2(win)		for (auto iter = win.getIterator(); iter.currentItem() != nullptr; iter.next())



namespace dis
{
	/**********************************************************************************************************/

	class WeightedCountArray
	{
		std::vector<int> innerArray;
		vector<float> weights;
		float weightSum;
	public:
		WeightedCountArray(vector<float> weights) : weights(weights)
		{
			size_t capacity = weights.size();
			innerArray = vector<int>(capacity, 0); // creates a vector of capacity size, all elements with value 0
			weightSum = 0;
		}

		WeightedCountArray(int capacity) : WeightedCountArray(vector<float>(capacity, 1))
		{
		}

		void SetCount(int index, int newValue) {
			//if ( index>=innerArray.size() || index<0 ){
			//    return;
			//}

			if (innerArray[index] == 0 && newValue != 0)
			{
				float weight = weights[index];
				weightSum += weight;

			}
			else if (innerArray[index] != 0 && newValue == 0)
			{
				float weight = weights[index];
				weightSum -= weight;
			}

			innerArray[index] = newValue;
		}

		int GetCount(int index) const {
			/*if ( index>=innerArray.size() || index<0 ){
			return 0;
			}*/
			return innerArray[index];
		}

		float GetNonZeroElementsWeight() const { return weightSum; }

		void Increase(const int index)
		{
			SetCount(index, GetCount(index) + 1);
		}
		
		void Decrease(const int index)
		{
			SetCount(index, GetCount(index) - 1);
		}

		void CountFrom(const Window<int>& win)
		{
			FOREACH_COLUMN_SCAN(j, i, win)
			{
				const int& element = win(i, j);
				Increase(element);
			}
		}

		void ReverseCountFrom(const Window<int>& win)
		{
			FOREACH_COLUMN_SCAN(j, i, win)
			{
				const int& element = win(i, j);
				Decrease(element);
			}
		}
	};

	/**********************************************************************************************************/

	class CountArray
	{
		std::vector<int> innerCountArray;
		int totalCount;
	public:
		CountArray(int capacity) : totalCount(0)
		{
			innerCountArray = vector<int>(capacity, 0); // creates a vector of capacity size, all elements with value 0
		}

		int Increase(int index)
		{
			int& count = innerCountArray[index];
			if (count == 0) ++totalCount;
			++count;
			DEBUG_ASSERT(count >= 0);
			DEBUG_ASSERT(totalCount >= 0);
			return count;
		}

		int Decrease(int index){
			int& count = innerCountArray[index];
			if (count == 1) --totalCount;
			--count; 
			DEBUG_ASSERT(count >= 0);
			DEBUG_ASSERT(totalCount >= 0);
			return count;
		}

		void Set(int index, int newValue) {
			//if ( index>=innerCountArray.size() || index<0 ){
			//    return;
			//}
			int& count = innerCountArray[index];
			if (count == 0 && newValue != 0)
			{
				totalCount += 1;
			}
			else if (count != 0 && newValue == 0)
			{
				totalCount -= 1;
			}

			count = newValue;
			DEBUG_ASSERT(count >= 0);
			DEBUG_ASSERT(totalCount >= 0);
		}

		int GetCount(int index) const {
			/*if ( index>=innerCountArray.size() || index<0 ){
			return 0;
			}*/
			return innerCountArray[index];
		}

		int GetNonZeroElementsCount() const { return totalCount; }

		void CountFrom(const Window<int>& win)
		{
			FOREACH_COLUMN_SCAN(j, i, win)
			{
				const int& element = win(i, j);
				Increase(element);
			}
		}
		
		void ReverseCountFrom(const Window<int>& win)
		{
			FOREACH_COLUMN_SCAN(j, i, win)
			{
				const int& element = win(i, j);
				Decrease(element);
			}
		}
	};

	/**********************************************************************************************************/

	class DeformableDiversityWeightArray : private CountArray
	{
		std::vector<double> innerWeightArray_;
		double h_;
		double weightIncreaseFactor_;
		double weightDecreaseFactor_;
	public:
		DeformableDiversityWeightArray(int capacity, double h) : CountArray(capacity)
		{
			h_ = h;
			weightIncreaseFactor_ = exp(-1 / h);
			weightDecreaseFactor_ = exp(+1 / h);
			double weight_0 = weightDecreaseFactor_; // exp(1-t / h) where t==0
			innerWeightArray_ = vector<double>(capacity, weight_0); // creates a vector of capacity size, all elements with value exp(1-t / h) where t==0
		}

		void Increase(int index)
		{
			CountArray::Increase(index);
			innerWeightArray_[index] *= weightIncreaseFactor_;
			DEBUG_ASSERT(innerWeightArray_[index] - exp((1 - CountArray::GetCount(index)) / h_)  <   0.001);
		}

		void Decrease(int index)
		{
			CountArray::Decrease(index);
			innerWeightArray_[index] *= weightDecreaseFactor_;
			DEBUG_ASSERT(innerWeightArray_[index] - exp((1 - CountArray::GetCount(index)) / h_)  <   0.001);
		}

		double GetWeight(int index) const
		{
			return innerWeightArray_[index];
		}

		void CountFrom(const Window<int>& win)
		{
			FOREACH_COLUMN_SCAN(j, i, win)
			{
				const int& element = win(i, j);
				Increase(element);
			}
		}

		void ReverseCountFrom(const Window<int>& win)
		{
			FOREACH_COLUMN_SCAN(j, i, win)
			{
				const int& element = win(i, j);
				Decrease(element);
			}
		}
	};
}


#undef FOREACH_COLUMN_SCAN