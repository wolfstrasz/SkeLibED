#ifndef _TEST_GOL_HPP
#define _TEST_GOL_HPP

#include <iostream>
#include <string>
#include <vector>
#include <utility>
#include <chrono>
#include <iostream>
#include <fstream>
#include <math.h>
#include "../Skeletons/Stencil.hpp"
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

// Test running and checking
// ----------------------------------------------------------

void initGolPattern(int pat_size) {
	golPattern = Pattern();
	for (int i = 1 - pat_size; i < pat_size; i++) {
		for (int j = 1 - pat_size; j < pat_size; j++) {
			golPattern.add(i, j);
		}
	}
}

void initGOL(int xdim, int ydim) {
	int xItem = (xdim / 2);
	int yItem = (ydim / 2);

	golIn = std::vector<GoL>(xdim * ydim);
	golOut = std::vector<GoL>(xdim * ydim);

	golIn.at(xItem * ydim + yItem) = GoL(true);
	xItem--;
	golIn.at(xItem * ydim + yItem) = GoL(true);
	yItem--;
	golIn.at(xItem * ydim + yItem) = GoL(true);

}
void goltest(int threads, int xdim, int ydim, int iter, int pat_size) {
	
	double overalltime = 0.0f;
	int tests = 1;

	initGolPattern(pat_size);
	initGOL(xdim, ydim);
	auto golStencil = Stencil(threads);
	
	// TEST NORMAL GOL TIME
	overalltime = 0.0f;
	for (int t = 0; t < tests; t++) {
		initGOL(xdim, ydim);
		auto start = std::chrono::system_clock::now();
		for (int i = 0; i < iter; i += 2) {

			// iterate forwards
			golStencil(golOut, golIn, golPattern, PSLED_NORMAL, xdim, ydim);

			// iterate backwards
			golStencil(golIn, golOut, golPattern, PSLED_NORMAL, xdim, ydim);

		}
		auto end = std::chrono::system_clock::now();
		std::chrono::duration<double, std::milli> time = end - start;
		overalltime += time.count();
	}
	std::cout << "Stencil::NORMAL::GOL::" << std::to_string(overalltime / tests) << std::endl;

	
	// TEST NORMAL OPT 3 GOL TIME
	overalltime = 0.0f;
	for (int t = 0; t < tests; t++) {
		std::cout << "Test (" << t << ")...\n";
		initGOL(xdim, ydim);
		auto start = std::chrono::system_clock::now();
		for (int i = 0; i < iter; i += 2) {

			// iterate forwards
			golStencil(golOut, golIn, golPattern, PSLED_OPT_NORMAL, xdim, ydim);

			// iterate backwards
			golStencil(golIn, golOut, golPattern, PSLED_OPT_NORMAL, xdim, ydim);
		}
		auto end = std::chrono::system_clock::now();
		std::chrono::duration<double, std::milli> time = end - start;
		overalltime += time.count();
	}

	std::cout << "Stencil::OPT_NOTMAL::GOL::" << std::to_string(overalltime / tests) << std::endl;

}

#endif // !_TEST_GOL_HPP
