#include "main.h"
#include "mex.h"
#include "../mex_opencv_2.4/MxArray.hpp"
#include "../mex_opencv_2.4/EigenExtensions.h"
#include "mexInterface.h"
#include "diversity.h"
#include "deformableDiversity.h"
#include "tictoc.h"

using namespace std;
using namespace dis;

struct DiversityTypes {
	static const char* DiversityCount;
	static const char* WeightedDiversityCount;
	static const char* DeformableDiversity;
	static vector<const char*> All;
};

// this needs to be placed in a single translation unit only
const char* DiversityTypes::DiversityCount = "DiversityCount";
const char* DiversityTypes::WeightedDiversityCount = "weightedDiversityCount";
const char* DiversityTypes::DeformableDiversity = "DeformableDiversity";
vector<const char*> DiversityTypes::All = { DiversityCount, WeightedDiversityCount, DeformableDiversity };


template <typename T>
string stringJoin(string delim, vector<T> objs)
{

	if (objs.size() <= 0) return "";
	std::string result = objs[0];
	for (int i = 1; i < objs.size(); ++i)
	{
		T o = objs[i];
		result += delim;
		result += o;
	}

	return result;
}

string excapsulate(string enc, string str)
{
	return enc + str + enc;
}

string BuildInputErrMessage()
{
	ostringstream stringStream;
	stringStream
		<< "Expected inputs: " << "\n"
		<< excapsulate("'",DiversityTypes::DiversityCount) << ", nnfMat, windowM, windowN, differentWeightStartIndex[optional], differentWeight[optional]" << "\n"
		<< excapsulate("'", DiversityTypes::WeightedDiversityCount) << ", nnfMat, windowM, windowN, weights" << "\n"
		<< excapsulate("'", DiversityTypes::DeformableDiversity) << ", nnfMat, windowM, windowN, xyPositions, [optional]h" << "\n";
		
	string message = stringStream.str().c_str();
	return message;
}


void finishWithTimiings(double secs)
{
	ostringstream s;
	s << "elapsed time in seconds: " << secs << "\n";
	mexPrintf(s.str().c_str());
}

// Expose two available interfaces:
//	diversity('DiversityCount', nnfMat, windowM, windowN, differentWeightStartIndex[optional], differentWeight[optional]);
//  diversity('weightedDiversityCount', nnfMat, windowM, windowN, weights); 
//  diversity('DeformableDiversity', nnfMat, windowM, windowN, xyPositions, [optional]h); 
//  note that M represents rows and N represents columns
void mexFunction(int outputElementCount, mxArray *outputs[], int inputsElementCount, const mxArray *inputs[])
{
	tic();
	try
	{
		// verify signature of function
		ASSERT2(
			inputsElementCount >= Idx::IN_windowSizeN + 1, 
			BuildInputErrMessage().c_str());

		ASSERT2(
			outputElementCount == 1,
			"expected outpus: (diversity map)");

		string diversityType;
		try
		{
			diversityType = MxArray(inputs[Idx::IN_DIVERSITY_TYPE]).toString();
		}
		catch (...)
		{
			ERROR((string("First parameter should be a string. ") + BuildInputErrMessage()).c_str());
		}

		string::const_pointer countbased = DiversityTypes::DiversityCount;
		if (diversityType == countbased)
		{
			DiversityCount(outputs, inputsElementCount, inputs);
			return;
		}

		string::const_pointer weightedcounting = DiversityTypes::WeightedDiversityCount;
		if (diversityType == weightedcounting)
		{
			WeightedDiversityCount(outputs, inputsElementCount, inputs);
			return;
		}

		string::const_pointer radiusbasedweighting = DiversityTypes::DeformableDiversity;
		if (diversityType == radiusbasedweighting)
		{
			DeformableDiversity(outputs, inputsElementCount, inputs);
			//finishWithTimiings(toc());
			return;
		}

		string delim = "/";
		std::string errMessage = "first parameter should be one of the following options: ";
		errMessage += stringJoin(delim, DiversityTypes::All);
		ERROR(errMessage.c_str());

		auto nnfMat2 = mexArray2EigenMat_int(MxArray(inputs[Idx::IN_NNF]));

	}
	catch (int e)
	{
		ostringstream s;
		s << "An exception occurred. Exception Nr. " << e << '\n';
		ERROR(s.str().c_str());
	}
}

