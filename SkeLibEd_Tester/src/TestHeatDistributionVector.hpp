#ifndef _TEST_HEAT_DISTRIBUTION_VECTOR_HPP
#define _TEST_HEAT_DISTRIBUTION_VECTOR_HPP

#include "Pixel.hpp"
#include "Stencil.hpp"
#include <chrono>
#include <iostream>
#include <fstream>
#include <string>
#include <math.h>
/* uses Jacobi method to solve a sparse system of linear equations */
/* to solve we need conditions at the border to be kept constant */
#define MAX_PLATE_DIMV 1024
std::vector<float> boardIn = std::vector<float>(MAX_PLATE_DIMV * MAX_PLATE_DIMV);		// iterative simulation would require us to switch betweeen Input and Output
std::vector<float> boardOut = std::vector<float>(MAX_PLATE_DIMV * MAX_PLATE_DIMV);
Pattern heatPattern;


void initPattern() {
	// Initialise heat pattern
	heatPattern.add(-1, 0);
	heatPattern.add(0, 1);
	heatPattern.add(1, 0);
	heatPattern.add(0, -1);
}

void initPlate() {
	// Initialise start plate cold with blue value (1.0f)
	for (int i = 0; i < MAX_PLATE_DIMV; i++) {
		//	std::cout << i << std::endl;
		for (int j = 0; j < MAX_PLATE_DIMV; j++) {
			//	std::cout << j << std::endl;
			boardIn.at(i * MAX_PLATE_DIMV + j) = 1.0f;
			boardOut.at(i * MAX_PLATE_DIMV + j) = 1.0f;
		}
	}
	// Initialise hot spot (from below and top the plate) with red value (3.0f);
	for (int j = 1; j < MAX_PLATE_DIMV - 1; j++) {
		boardIn.at((MAX_PLATE_DIMV - 1)*MAX_PLATE_DIMV + j) = 3.0f;
		boardIn.at(j) = 3.0f;

		boardOut.at((MAX_PLATE_DIMV - 1)*MAX_PLATE_DIMV + j) = 3.0f;
		boardOut.at(j) = 3.0f;
	}
}

psled::pPixel convertToHeatMap(float value, float minimum = 1.0f, float maximum = 3.0f) {
	psled::pPixel k;
	float halfmax = (minimum + maximum) / 2;
	float ratio = 2 * (value - minimum) / (maximum - minimum);

	k.b = (int)(std::max(0.0f, 255 * (1.0f - ratio)));
	k.r = (int)(std::max(0.0f, 255 * (ratio - 1.0f)));
	k.g = 255 - k.b - k.r;
	return k;
}


void printPPM(std::string name, int iters, int threads) {
	std::string outfileName = "vheat_" + name + "_" + std::to_string(threads) + "_" + std::to_string(iters) + ".ppm";
	FILE *outfile;
	outfile = fopen(outfileName.c_str(), "w");
	fprintf(outfile, "P6\n");
	fprintf(outfile, "%d %d\n%d\n", MAX_PLATE_DIMV, MAX_PLATE_DIMV, 255);

	psled::pPixel heatPixel;
	for (int i = 0; i < MAX_PLATE_DIMV; i++) {
		for (int j = 0; j < MAX_PLATE_DIMV; j++) {
			heatPixel = convertToHeatMap(boardIn.at(i * MAX_PLATE_DIMV + j));
			fputc((char)heatPixel.r, outfile);
			fputc((char)heatPixel.g, outfile);
			fputc((char)heatPixel.b, outfile);
		}
	}

	fclose(outfile);
}
/* https://www.sciencedirect.com/topics/computer-science/stencil-pattern */
/* https://www.cs.uky.edu/~jzhang/CS621/chapter7.pdf */

void testHeatDistributionVector(int maxiters = 0, int threadcount = 0) {
	std::cout << "( " << maxiters << ", " << threadcount << ")\n";
	//std::cout <<" HEAT DISTRIB HARDWARE CONCURRENCY: " << std::thread::hardware_concurrency() << "\n";
	initPattern();
	//std::cout << "Initialised pattern\n";
	initPlate();
	//std::cout << "Initialised plate\n";
	threadcount = threadcount ? threadcount : std::thread::hardware_concurrency();

	//for (int i = 0; i < MAX_PLATE_DIM; i++) {
	//	for (int j = 0; j < MAX_PLATE_DIM; j++) {
	//		std::cout << boardIn[i][j] << "<>";
	//	}
	//	std::cout << "\n";
	//}

//	std::cout << "( " << maxiters << ", " << threadcount << ")\n";
	auto stencil = Stencil(threadcount);
	//std::cout << "SEQ TEST START\n";
	// TEST SQUENTIAL
	auto start = std::chrono::system_clock::now();

	for (int iter = 0; iter < maxiters; iter += 2) {

		// Forwards iteration IN --> OUT
		for (int i = 0; i < MAX_PLATE_DIMV; i++) {
			for (int j = 0; j < MAX_PLATE_DIMV; j++) {
				// Check for border
				if (i < -heatPattern.getRowLowerBoundary()
					|| i >= (MAX_PLATE_DIMV - heatPattern.getRowHigherBoundary())
					|| j < -heatPattern.getColumnLowerBoundary()
					|| j >= (MAX_PLATE_DIMV - heatPattern.getColumnHigherBoundary())
					) {
					boardOut.at(MAX_PLATE_DIMV * i + j) = boardIn.at(MAX_PLATE_DIMV * i + j);
					continue;
				}
				// if not bordering add
				float sum = 0;
				for (int k = 0; k < heatPattern.size(); k++) {
					int ri = i + heatPattern.rowOffset(k);
					int ci = j + heatPattern.columnOffset(k);
					sum += boardIn.at(MAX_PLATE_DIMV * ri + ci);
				}
				sum = sum / heatPattern.size();
				boardOut.at (MAX_PLATE_DIMV * i + j) = sum;
			}
		}
		// Backwards iteration OUT --> IN
		for (int i = 0; i < MAX_PLATE_DIMV; i++) {
			for (int j = 0; j < MAX_PLATE_DIMV; j++) {
				// Check for border
				if (i < -heatPattern.getRowLowerBoundary()
					|| i >= (MAX_PLATE_DIMV - heatPattern.getRowHigherBoundary())
					|| j < -heatPattern.getColumnLowerBoundary()
					|| j >= (MAX_PLATE_DIMV - heatPattern.getColumnHigherBoundary())
					) {
					boardIn.at(MAX_PLATE_DIMV * i + j) = boardOut.at(MAX_PLATE_DIMV * i + j);
					continue;
				}
				// if not bordering add
				float sum = 0;
				for (int k = 0; k < heatPattern.size(); k++) {
					int ri = i + heatPattern.rowOffset(k);
					int ci = j + heatPattern.columnOffset(k);
					sum += boardOut.at(MAX_PLATE_DIMV * ri + ci);
				}
				sum = sum / heatPattern.size();
				boardIn.at(MAX_PLATE_DIMV * i + j) = sum;
			}
		}


	}

	auto end = std::chrono::system_clock::now();
	std::chrono::duration<double, std::milli> time = end - start;
	std::cout << "Stencil::Sequential::" << std::to_string(time.count()) << std::endl;

	std::cout << "PARALLEL TEST START\n";

	//for (int i = 0; i < MAX_PLATE_DIM; i++) {
	//	for (int j = 0; j < MAX_PLATE_DIM; j++) {
	//		std::cout << boardIn[i][j] << "<>";
	//	}
	//	std::cout << "\n";
	//}

	printPPM("Sequential", maxiters, threadcount);


	initPlate();
	// TEST PARALLEL
	start = std::chrono::system_clock::now();
	for (int iter = 0; iter < maxiters; iter += 2) {
		// Forwards iteration
		stencil(boardOut, boardIn, heatPattern, PSLED_BORDER, MAX_PLATE_DIMV, MAX_PLATE_DIMV);
		// Backwards iteration
		stencil(boardIn, boardOut, heatPattern, PSLED_BORDER, MAX_PLATE_DIMV, MAX_PLATE_DIMV);
	}
	end = std::chrono::system_clock::now();
	printPPM("Parallel", maxiters, threadcount);

	time = end - start;
	std::cout << "Stencil::Parallel::" << std::to_string(time.count()) << std::endl;


}

#endif // !_TEST_HEAT_DISTRIBUTION_VECTOR_HPP
