#pragma once

#include "mex.h"

namespace dis
{
	void WeightedDiversityCount(mxArray** outputs, int inputsElementCount, const mxArray** inputs);
	void DiversityCount(mxArray** outputs, int inputsElementCount, const mxArray** inputs);
}