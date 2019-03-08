#include <iostream>
#include <limits.h>
#include <chrono>
#include <fstream>

//#include "TestCollatz.hpp"
//#include "TestMandelbrot.hpp"
//#include "TestNBody.hpp"
//#include "TestStencil.hpp"
//#include "mandelbrot.hpp"
//#include "TestHeatDistribution.hpp"
#include "TestStencilVector.hpp"
#include "TestHeatDistributionVector.hpp"
int main(int argc, char* argv[]) {

	size_t func = 0;
	size_t thrc = 0;
	size_t blkc = 0;
	size_t ic = 0;
	size_t iters = 0;
	size_t ic2 = 0;
	double arg = 0.0f;
	//--------------------------------------------
	int index = 1;
	func = strtol(argv[index++], nullptr, 0);

	if (func == -1) {
		std::cout << "RUNNING: TEST STENCIL\n";
		//teststencil::test();
		return 0;
	}
	if (func == -2) {
		std::cout << "RUNNING: TEST STENCIL VECTOR\n";
		testStencilVector();
		return 0;
	}
	thrc = strtol(argv[index++], nullptr, 0);
	if (func != 5)
	blkc = strtol(argv[index++], nullptr, 0);
	if (func != 5)
	ic = strtol(argv[index++], nullptr, 0);
	if (func != 1) {
		iters = strtol(argv[index++], nullptr, 0);
	}
	if (func != 3 && func != 5) {
		arg = strtod(argv[index++], nullptr);
	}
	//	if (func == 4){
		//	ic2 = strtol(argv[index++], nullptr, 0);
		//}
	std::cout << "TEST:    " << func << std::endl;		// test functionality
	std::cout << "THREADS: " << thrc << std::endl;		// number of threads
	std::cout << "BLOCKS:  " << blkc << std::endl;		// number of blocks
	std::cout << "IC:      " << ic << std::endl;		// number of items in a dimension
	std::cout << "ITERS:   " << iters << std::endl;		// number of iterations
	std::cout << "ARG:     " << arg << std::endl;		// additional arg
	if (func == 1) {
		//	collatz::test(thrc, blkc, ic, arg);
	}
	else if (func == 2) {
		//	mandelbrot::test(thrc, blkc, ic, ic, iters, arg);
	}
	else if (func == 3) {
		//	nbody::test(thrc, blkc, ic, iters);
	}
	else if (func == 4) {
		//	draw(thrc, blkc, ic, ic, iters, arg);
	}
	else if (func == 5){
		//testHeatDistribution(iters, thrc);
		testHeatDistributionVector(iters, thrc);
	}

	return 0;
}
