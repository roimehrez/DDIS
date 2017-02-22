#include "tictoc.h"

#include <stack>
#include <ctime>

std::stack<clock_t> tictoc_stack;

void dis::tic() {
	tictoc_stack.push(clock());
}

// return seconds
double dis::toc() {
	return (double)(clock() - tictoc_stack.top()) / CLOCKS_PER_SEC;
}