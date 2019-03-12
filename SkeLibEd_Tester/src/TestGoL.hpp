#ifndef _TEST_GOL_HPP
#define _TEST_GOL_HPP

#define GOLX 1000
#define GOLY 5000
#define GOLTHREADS 4
#define GOLITERS 1
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

std::vector<GoL> golIn = std::vector<GoL>(GOLX * GOLY);
std::vector<GoL> golOut = std::vector<GoL>(GOLX * GOLY);
Pattern golPattern;


void golPrintOut() {
	std::cout << "GOL OUT: \n";
	for (int i = 0; i < GOLX; i++) {
		for (int j = 0; j < GOLY; j++) {
			if (golOut.at(i * GOLY + j).alive) std::cout << 1 << " ";
			else std::cout << 0 << " ";
		}
		std::cout << std::endl;
	}
}

void golPrintIn() {
	std::cout << "GOL IN: \n";
	for (int i = 0; i < GOLX; i++) {
		for (int j = 0; j < GOLY; j++) {
			if (golIn.at(i * GOLY + j).alive) std::cout << 1 << " ";
			else std::cout << 0 << " ";
		}
		std::cout << std::endl;
	}
}

#define PATTERNMAXSIZE 3

void initGolPattern() {
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
	for (int i = 1 - PATTERNMAXSIZE; i < PATTERNMAXSIZE; i++) {
		for (int j = 1 - PATTERNMAXSIZE; j < PATTERNMAXSIZE; j++) {
			golPattern.add(i, j);
		}
	}
}

void initGOL() {
	golIn = std::vector<GoL>(GOLX * GOLY);
	golOut = std::vector<GoL>(GOLX * GOLY);
	int xItem = (GOLX / 2);
	int yItem = (GOLY / 2);
	golIn.at(xItem * GOLY + yItem) = GoL(true);
	xItem--;
	golIn.at(xItem * GOLY + yItem) = GoL(true);
	yItem--;
	golIn.at(xItem * GOLY + yItem) = GoL(true);
	//golPrintIn();
	//golPrintOut();
}
void goltest() {
	
	initGolPattern();
	initGOL();

	
	auto golStencil = Stencil(GOLTHREADS);

	// TEST NORMAL GOL TIME
	//std::cout << "RUNNING NORMAL GOL" << std::endl;
	auto start = std::chrono::system_clock::now();
	for (int i = 0; i < GOLITERS; i += 2) {

		// iterate forwards
		golStencil(golOut, golIn, golPattern, PSLED_NORMAL, GOLX, GOLY);

	//	golPrintOut();
		// iterate backwards
		golStencil(golIn, golOut, golPattern, PSLED_NORMAL, GOLX, GOLY);
		//golPrintIn();

	}
	auto end = std::chrono::system_clock::now();
	std::chrono::duration<double, std::milli> time = end - start;
	std::cout << "Stencil::NORMAL::GOL::" << std::to_string(time.count()) << std::endl;

	//golPrintIn();

	initGOL();

	// TEST NORMAL OPT 1 GOL TIME
	//std::cout << "RUNNING NORMAL OPT GOL" << std::endl;

	start = std::chrono::system_clock::now();
	for (int i = 0; i < GOLITERS; i += 2) {

		//std::cout << "FIRST ITER\n";
		// iterate forwards
		golStencil(golOut, golIn, golPattern, PSLED_NORMAL_OPT1, GOLX, GOLY);

	//	std::cout << "Second ITER\n";
		//	golPrintOut();
			// iterate backwards
		golStencil(golIn, golOut, golPattern, PSLED_NORMAL_OPT1, GOLX, GOLY);
		//golPrintIn();

	}
	end = std::chrono::system_clock::now();
	time = end - start;
	std::cout << "Stencil::NORMAL_OPT_!::GOL::" << std::to_string(time.count()) << std::endl;

	//golPrintIn();

}

#endif // !_TEST_GOL_HPP
