#ifndef _TEST_GOL_HPP
#define _TEST_GOL_HPP

#define GOLX 1000
#define GOLY 5000
#define GOLTHREADS 4
#define GOLITERS 1
#define MAX_TESTS_GOL 10
#include <iostream>
#include <string>
#include <vector>
#include <utility>
#include <chrono>
#include <iostream>
#include <fstream>
#include <math.h>
#include "Stencil.hpp"
#include "GameOfLife.hpp"

std::vector<GoL> golIn;
std::vector<GoL> golOut;
Pattern golPattern;


void golPrintOut(int xdim, int ydim) {
	std::cout << "GOL OUT: \n";
	for (int i = 0; i < xdim; i++) {
		for (int j = 0; j < ydim; j++) {
			if (golOut.at(i * ydim + j).alive) std::cout << 1 << " ";
			else std::cout << 0 << " ";
		}
		std::cout << std::endl;
	}
}

void golPrintIn(int xdim, int ydim) {
	std::cout << "GOL IN: \n";
	for (int i = 0; i < xdim; i++) {
		for (int j = 0; j < ydim; j++) {
			if (golIn.at(i * ydim + j).alive) std::cout << 1 << " ";
			else std::cout << 0 << " ";
		}
		std::cout << std::endl;
	}
}


void initGolPattern(int pat_size) {
	//golPattern.add(-1, -1);
	//golPattern.add(-1, 0);
	//golPattern.add(-1, 1);
	//golPattern.add(0, -1);
	//golPattern.add(0, 1);
	//golPattern.add(1, -1);
	//golPattern.add(1, 0);
	//golPattern.add(1, 1);

	// INCREASE
	golPattern = Pattern();
	for (int i = 1 - pat_size; i < pat_size; i++) {
		for (int j = 1 - pat_size; j < pat_size; j++) {
			golPattern.add(i, j);
		}
	}
}

void initGOL(int xdim, int ydim) {
	golIn = std::vector<GoL>(xdim * ydim);
	golOut = std::vector<GoL>(xdim * ydim);
	int xItem = (xdim / 2);
	int yItem = (ydim / 2);
	golIn.at(xItem * ydim + yItem) = GoL(true);
	xItem--;
	golIn.at(xItem * ydim + yItem) = GoL(true);
	yItem--;
	golIn.at(xItem * ydim + yItem) = GoL(true);
	//golPrintIn();
	//golPrintOut();
}
void goltest(int threads, int xdim, int ydim, int iter, int pat_size) {
	
	initGolPattern(pat_size);
	initGOL(xdim, ydim);
	auto golStencil = Stencil(threads);
	
	double overalltime = 0.0f;
	int tests = MAX_TESTS_GOL;

	//golPrintIn(xdim,ydim);
	//golPrintOut(xdim, ydim);
	overalltime = 0.0f;
	// TEST NORMAL BORDER GOL TIME
	//std::cout << "RUNNING NORMAL OPT GOL" << std::endl;
	for (int t = 0; t < tests; t++) {
		std::cout << "Test (" << t << ")...\n";
		initGOL(xdim, ydim);
		auto start = std::chrono::system_clock::now();
		for (int i = 0; i < iter; i += 2) {

			// iterate forwards
			golStencil(golOut, golIn, golPattern, PSLED_BORDER, xdim, ydim);

			//	golPrintOut();
				// iterate backwards
			golStencil(golIn, golOut, golPattern, PSLED_BORDER, xdim, ydim);
			//golPrintIn();

		}
		auto end = std::chrono::system_clock::now();
		std::chrono::duration<double, std::milli> time = end - start;
		overalltime += time.count();
	}

	std::cout << "Stencil::BORDER::GOL::" << std::to_string(overalltime / tests) << std::endl;
	// TEST NORMAL GOL TIME
	//std::cout << "RUNNING NORMAL GOL" << std::endl;
	overalltime = 0.0f;
	for (int t = 0; t < tests; t++) {
		std::cout << "Test (" << t << ")...\n";
		initGOL(xdim, ydim);
		auto start = std::chrono::system_clock::now();
		for (int i = 0; i < iter; i += 2) {

			// iterate forwards
			golStencil(golOut, golIn, golPattern, PSLED_NORMAL, xdim, ydim);
	//		golPrintOut(xdim, ydim);

			// iterate backwards
			golStencil(golIn, golOut, golPattern, PSLED_NORMAL, xdim, ydim);
	//		golPrintIn(xdim,ydim);

		}
		auto end = std::chrono::system_clock::now();
		std::chrono::duration<double, std::milli> time = end - start;
		overalltime += time.count();
	}
	std::cout << "Stencil::NORMAL::GOL::" << std::to_string(overalltime / tests) << std::endl;

	//golPrintIn(xdim, ydim);

	
	overalltime = 0.0f;
	// TEST NORMAL OPT 1 GOL TIME
	//std::cout << "RUNNING NORMAL OPT GOL" << std::endl;
	for (int t = 0; t < tests; t++) {
		std::cout << "Test (" << t << ")...\n";
		initGOL(xdim, ydim);
		auto start = std::chrono::system_clock::now();
		for (int i = 0; i < iter; i += 2) {

			// iterate forwards
			golStencil(golOut, golIn, golPattern, PSLED_NORMAL_OPT1, xdim, ydim);

			//	golPrintOut();
				// iterate backwards
			golStencil(golIn, golOut, golPattern, PSLED_NORMAL_OPT1, xdim, ydim);
			//golPrintIn();

		}
		auto end = std::chrono::system_clock::now();
		std::chrono::duration<double, std::milli> time = end - start;
		overalltime += time.count();
	}
	std::cout << "Stencil::NORMAL_OPT_1::GOL::" << std::to_string(overalltime / tests) << std::endl;

//	golPrintIn(xdim,ydim);


	overalltime = 0.0f;
	// TEST NORMAL OPT 2 GOL TIME
	//std::cout << "RUNNING NORMAL OPT GOL" << std::endl;
	for (int t = 0; t < tests; t++) {
		std::cout << "Test (" << t << ")...\n";
		initGOL(xdim, ydim);
		auto start = std::chrono::system_clock::now();
		for (int i = 0; i < iter; i += 2) {

			// iterate forwards
			golStencil(golOut, golIn, golPattern, PSLED_NORMAL_OPT2, xdim, ydim);

			//	golPrintOut();
				// iterate backwards
			golStencil(golIn, golOut, golPattern, PSLED_NORMAL_OPT2, xdim, ydim);
			//golPrintIn();

		}
		auto end = std::chrono::system_clock::now();
		std::chrono::duration<double, std::milli> time = end - start;
		overalltime += time.count();
	}
	std::cout << "Stencil::NORMAL_OPT_2::GOL::" << std::to_string(overalltime / tests) << std::endl;
//	golPrintIn(xdim,ydim);


	overalltime = 0.0f;
	// TEST NORMAL OPT 3 GOL TIME
	//std::cout << "RUNNING NORMAL OPT GOL" << std::endl;
	for (int t = 0; t < tests; t++) {
		std::cout << "Test (" << t << ")...\n";
		initGOL(xdim, ydim);
		auto start = std::chrono::system_clock::now();
		for (int i = 0; i < iter; i += 2) {

			// iterate forwards
			golStencil(golOut, golIn, golPattern, PSLED_NORMAL_OPT3, xdim, ydim);

			//	golPrintOut();
				// iterate backwards
			golStencil(golIn, golOut, golPattern, PSLED_NORMAL_OPT3, xdim, ydim);
			//golPrintIn();

		}
		auto end = std::chrono::system_clock::now();
		std::chrono::duration<double, std::milli> time = end - start;
		overalltime += time.count();
	}

	std::cout << "Stencil::NORMAL_OPT_3::GOL::" << std::to_string(overalltime / tests) << std::endl;
	//golPrintIn(xdim,ydim);

	overalltime = 0.0f;
	// TEST NORMAL OPT 4 GOL TIME
	//std::cout << "RUNNING NORMAL OPT GOL" << std::endl;
	for (int t = 0; t < tests; t++) {
		std::cout << "Test (" << t << ")...\n";
		initGOL(xdim, ydim);
		auto start = std::chrono::system_clock::now();
		for (int i = 0; i < iter; i += 2) {

			// iterate forwards
			golStencil(golOut, golIn, golPattern, PSLED_NORMAL_OPT4, xdim, ydim);

			//	golPrintOut();
				// iterate backwards
			golStencil(golIn, golOut, golPattern, PSLED_NORMAL_OPT4, xdim, ydim);
			//golPrintIn();

		}
		auto end = std::chrono::system_clock::now();
		std::chrono::duration<double, std::milli> time = end - start;
		overalltime += time.count();
	}

	std::cout << "Stencil::NORMAL_OPT_4::GOL::" << std::to_string(overalltime / tests) << std::endl;


	
	//golPrintIn(xdim,ydim);

}

#endif // !_TEST_GOL_HPP
