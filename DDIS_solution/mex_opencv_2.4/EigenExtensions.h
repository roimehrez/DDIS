#pragma once

#include <Eigen/Core>
#include "MxArray.hpp"
#include "MexAsserts.h"

using namespace Eigen;
namespace Eigen{
	typedef Eigen::Matrix<unsigned, Eigen::Dynamic, Eigen::Dynamic>    MatrixXui;
	typedef Eigen::Matrix<unsigned short, Eigen::Dynamic, Eigen::Dynamic>    MatrixXui16;
	typedef Eigen::Matrix<short, Eigen::Dynamic, Eigen::Dynamic>    MatrixXi16;
}

template<typename _Scalar>
inline Map< Matrix<_Scalar, Dynamic, Dynamic>> mexArray2EigenMat(MxArray& mxObj)
{
	const mwSize cols = mxObj.cols();
	const mwSize rows = mxObj.rows();
	auto map = Map< Matrix<_Scalar, Dynamic, Dynamic>>((_Scalar*)mxObj.rawData(), rows, cols);
	return map;
}

#define CONVERSIONFUNC_MEXARRAY_2_EIGENMAT(type, headerName, returnedType, assertion) \
inline Map< returnedType > mexArray2EigenMat_##headerName(MxArray& mxObj) \
{\
	ASSERT(assertion);\
	return mexArray2EigenMat<type>(mxObj);\
}

CONVERSIONFUNC_MEXARRAY_2_EIGENMAT(double, double, MatrixXd, mxObj.isDouble())
CONVERSIONFUNC_MEXARRAY_2_EIGENMAT(int, int, MatrixXi, mxObj.isInt32())
CONVERSIONFUNC_MEXARRAY_2_EIGENMAT(short, int16, MatrixXi16, mxObj.isInt16())
CONVERSIONFUNC_MEXARRAY_2_EIGENMAT(unsigned, uint, MatrixXui, mxObj.isUint32())
CONVERSIONFUNC_MEXARRAY_2_EIGENMAT(unsigned short, uint16, MatrixXui16, mxObj.isUint16())

#define NTHREADS (std::max((int)std::thread::hardware_concurrency(), 4))

//#ifdef _DEBUG
#define printMat(mat)																				\
				{																					\
					stringstream stringstream;														\
					stringstream << #mat << " = " << std::endl << mat << std::endl << std::endl << flush;	\
					const std::string& tmp = stringstream.str();									\
					const char* cstr = tmp.c_str();													\
					mexPrintf(cstr);																\
				}

//#else
//#define printMat(mat)
//#endif

#define printMsg(msg)																				\
								{																	\
					stringstream stringstream;														\
					stringstream << msg << flush;													\
					const std::string& tmp = stringstream.str();									\
					const char* cstr = tmp.c_str();													\
					mexPrintf(cstr);																\
								}

