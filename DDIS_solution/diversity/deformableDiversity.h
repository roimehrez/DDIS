#pragma once

#include "mex.h"

namespace dis
{
	void DeformableDiversity(mxArray** outputs, int inputsElementCount, const mxArray** inputs, unsigned nthreads = 0);
}