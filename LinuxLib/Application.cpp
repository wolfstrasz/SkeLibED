#include <iostream>
#include <limits.h>
#include <chrono>
#include <fstream>

#include "Tests/NBody.hpp"
#include "Tests/Collatz.hpp"
#include "Tests/Mandelbrot.hpp"
#include "Tests/StencilUnitTest.hpp"
#include "Tests/TestHeatDistribution.hpp"
#include "Tests/TestImageProcessing.hpp"
#include "Tests/TestGoL.hpp"

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

	if (argv[index] == nullptr) {
		printf("Incorrect function input!\n");
		printf("Use one of the following sets of arguments for different testing\n");
		printf("Stencil 2D Array Unit tests:\r\t\t\t\t >> ./run.out -1 \n");
		printf("Stencil Vector Unit tests:  \r\t\t\t\t >> ./run.out -2 \n");
		printf("Collatz Map test:           \r\t\t\t\t >> ./run.out 1 \n");
		printf("Mandelbrot Map test:        \r\t\t\t\t >> ./run.out 2 \n");
		printf("Nbody Map test:             \r\t\t\t\t >> ./run.out 3 \n");
		printf("Heat equation test:         \r\t\t\t\t >> ./run.out 5 \n");
		printf("Game of Life test:          \r\t\t\t\t >> ./run.out 6 \n");
		printf("");
		return 0;
	}

	func = strtol(argv[index++], nullptr, 0);

	if (func == -1) {
	//	ts::test();
		return 0;
	}
	else if (func == -2) {
		tsv::test();
		return 0;
	}
	else if (func == -3) {
		test_img_pr::test();
		return 0;
	}
	else if (func == 1) {

		if (argv[index] == nullptr) {
			printf("Collatz test. Use args >> ./run.out 1 [threads] [blocks] [itemscount] [argument]\n");
			return 0;
		}
		thrc = strtol(argv[index++], nullptr, 0);
		blkc = strtol(argv[index++], nullptr, 0);
		ic = strtol(argv[index++], nullptr, 0);
		arg = strtod(argv[index++], nullptr);

		// Tests
		collatz::test(thrc, blkc, ic, arg);
	}
	else if (func == 2) {
		if (argv[index] == nullptr) {
			printf("Mandelbrot test. Use args >> ./run.out 2 [threads] [blocks] [array dim] [iterations] [magnification]\n");
			return 0;
		}
		thrc = strtol(argv[index++], nullptr, 0);
		blkc = strtol(argv[index++], nullptr, 0);
		ic = strtol(argv[index++], nullptr, 0);
		iters = strtol(argv[index++], nullptr, 0);
		arg = strtod(argv[index++], nullptr);

		// Tests
		mandelbrot::test(thrc, blkc, ic, ic, iters, arg);
		return 0;
	}
	else if (func == 3) {
		if (argv[index] == nullptr) {
			printf("Nbody test. Use args >> ./run.out 3 [threads] [blocks] [itemscount] [iterations]\n");
			return 0;
		}
		thrc = strtol(argv[index++], nullptr, 0);
		blkc = strtol(argv[index++], nullptr, 0);
		ic = strtol(argv[index++], nullptr, 0);
		iters = strtol(argv[index++], nullptr, 0);

		// Tests
		nbody::test(thrc, blkc, ic, iters);
	}
	else if (func == 4) {
		if (argv[index] == nullptr) {
			printf("Draw test. Use args >> ./run.out 4 [threads] [blocks] [itemscount] [iterations] [argument]\n");
			return 0;
		}

		thrc = strtol(argv[index++], nullptr, 0);
		blkc = strtol(argv[index++], nullptr, 0);
		ic = strtol(argv[index++], nullptr, 0);
		iters = strtol(argv[index++], nullptr, 0);
		arg = strtod(argv[index++], nullptr);

		// Tests
		//	draw(thrc, blkc, ic, ic, iters, arg);
	}
	else if (func == 5){
		if (argv[index] == nullptr) {
			printf("Head Distribution test. Use args >> ./run.out 5 [threads] [dimensions] [iterations]\n");
			return 0;
		}
		thrc = strtol(argv[index++], nullptr, 0);
		//blkc = strtol(argv[index++], nullptr, 0);
		ic = strtol(argv[index++], nullptr, 0);
		iters = strtol(argv[index++], nullptr, 0);

		// Tests
		//testHeatDistribution(blkc, ic, iters, thrc);
		thdv::testHeatDistributionVector(ic, iters, thrc);
	}
	else if (func == 6) {
		if (argv[index] == nullptr) {
			printf("Game of Life test. Use args >> ./run.out 6 [threads] [xdim] [ydim] [iterations] [pattern_radius]\n");
			return 0;
		}
		thrc = strtol(argv[index++], nullptr, 0);
		blkc = strtol(argv[index++], nullptr, 0);
		ic = strtol(argv[index++], nullptr, 0);
		iters = strtol(argv[index++], nullptr, 0);
		arg = strtod(argv[index++], nullptr);

		// Tests
		goltest(thrc, blkc, ic, iters, arg);
	}
	else {

		printf("Incorrect function input!\n");
		printf("Use one of the following sets of arguments for different testing\n");
		printf("Stencil 2D Array Unit tests:\r\t\t\t\t >> ./run.out -1 \n");
		printf("Stencil Vector Unit tests:  \r\t\t\t\t >> ./run.out -2 \n");
		printf("Collatz Map test:           \r\t\t\t\t >> ./run.out -2 \n");
		printf("Mandelbrot Map test:        \r\t\t\t\t >> ./run.out -2 \n");
		printf("Nbody Map test:             \r\t\t\t\t >> ./run.out -2 \n");
		printf("Heat equation test:         \r\t\t\t\t >> ./run.out -2 \n");
		printf("Game of Life test:          \r\t\t\t\t >> ./run.out -2 \n");
		printf("");
	}

	return 0;
}
