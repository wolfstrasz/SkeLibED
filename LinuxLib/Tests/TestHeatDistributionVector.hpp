#ifndef _TEST_HEAT_DISTRIBUTION_VECTOR_HPP
#define _TEST_HEAT_DISTRIBUTION_VECTOR_HPP

#include "Pixel.hpp"
#include "../Skeletons/Stencil.hpp"
#include <chrono>
#include <iostream>
#include <fstream>
#include <string>
#include <math.h>

namespace thdv {
#define thdv_test_count 10
	/* uses Jacobi method to solve a sparse system of linear equations */
	/* to solve we need conditions at the border to be kept constant */
//#define MAX_PLATE_DIMV 128
	std::vector<float> boardIn;	// iterative simulation would require us to switch betweeen Input and Output
	std::vector<float> boardOut;
	Pattern heatPattern;


	void initPattern() {
		// Initialise heat pattern
		heatPattern.add(-1, 0);
		heatPattern.add(0, 1);
		heatPattern.add(1, 0);
		heatPattern.add(0, -1);
	}

	void initPlate(int dim) {
		boardIn = std::vector<float>(dim * dim);
		boardOut = std::vector<float>(dim * dim);

		// Initialise start plate cold with blue value (1.0f)
		for (int i = 0; i < dim; i++) {
			//	std::cout << i << std::endl;
			for (int j = 0; j < dim; j++) {
				//	std::cout << j << std::endl;
				boardIn.at(i * dim + j) = 1.0f;
				boardOut.at(i * dim + j) = 1.0f;
			}
		}
		// Initialise hot spot (from below and top the plate) with red value (3.0f);
		for (int j = 1; j < dim - 1; j++) {
			boardIn.at((dim - 1)*dim + j) = 3.0f;
			boardIn.at(j) = 3.0f;

			boardOut.at((dim - 1)*dim + j) = 3.0f;
			boardOut.at(j) = 3.0f;
		}
	}

	psled::Pixel convertToHeatMap(float value, float minimum = 1.0f, float maximum = 3.0f) {
		psled::Pixel k;
		float halfmax = (minimum + maximum) / 2;
		float ratio = 2 * (value - minimum) / (maximum - minimum);

		k.b = (int)(std::max(0.0f, 255 * (1.0f - ratio)));
		k.r = (int)(std::max(0.0f, 255 * (ratio - 1.0f)));
		k.g = 255 - k.b - k.r;
		return k;
	}


	void printPPM(int dim, std::string name, int iters, int threads) {
		std::string outfileName = "vheat_" + name + "_" + std::to_string(threads) + "_" + std::to_string(iters) + ".ppm";
		FILE *outfile;
		outfile = fopen(outfileName.c_str(), "w");
		fprintf(outfile, "P6\n");
		fprintf(outfile, "%d %d\n%d\n", dim, dim, 255);

		psled::Pixel heatPixel;
		for (int i = 0; i < dim; i++) {
			for (int j = 0; j < dim; j++) {
				heatPixel = convertToHeatMap(boardIn.at(i * dim + j));
				fputc((char)heatPixel.r, outfile);
				fputc((char)heatPixel.g, outfile);
				fputc((char)heatPixel.b, outfile);
			}
		}

		fclose(outfile);
	}
	/* https://www.sciencedirect.com/topics/computer-science/stencil-pattern */
	/* https://www.cs.uky.edu/~jzhang/CS621/chapter7.pdf */

	void testHeatDistributionVector(int dim, int maxiters = 0, int threadcount = 0) {
		std::cout << "( " << maxiters << ", " << threadcount << ")\n";
		initPattern();
		auto stencil = Stencil(threadcount);



		//std::cout << "SEQ TEST START\n";
		// TEST SQUENTIAL

		//for (int t = 0; t < thdv_test_count; t++) {
		//  initPlate(dim);
		//	auto start = std::chrono::system_clock::now();
		//	for (int iter = 0; iter < maxiters; iter += 2) {
		//
		//		// Forwards iteration IN --> OUT
		//		for (int i = 0; i < dim; i++) {
		//			for (int j = 0; j < dim; j++) {
		//				// Check for border
		//				int factor = 0;
		//				if (i < -heatPattern.getRowLowerBoundary()
		//					|| i >= (dim - heatPattern.getRowHigherBoundary())
		//					|| j < -heatPattern.getColumnLowerBoundary()
		//					|| j >= (dim - heatPattern.getColumnHigherBoundary())
		//					) {
		//					boardOut.at(dim * i + j) = boardIn.at(dim * i + j);
		//					continue;
		//				}
		//				// if not bordering add
		//				float sum = 0;
		//				for (int k = 0; k < heatPattern.size(); k++) {
		//					int ri = i + heatPattern.rowOffset(k);
		//					int ci = j + heatPattern.columnOffset(k);
		//					sum += boardIn.at(dim * ri + ci) * heatPattern.itemWeight(k);
		//					factor += heatPattern.itemWeight(k);
		//				}
		//				sum = sum / factor;
		//				boardOut.at(dim * i + j) = sum;
		//			}
		//		}
		//		// Backwards iteration OUT --> IN
		//		for (int i = 0; i < dim; i++) {
		//			for (int j = 0; j < dim; j++) {
		//				// Check for border
		//				if (i < -heatPattern.getRowLowerBoundary()
		//					|| i >= (dim - heatPattern.getRowHigherBoundary())
		//					|| j < -heatPattern.getColumnLowerBoundary()
		//					|| j >= (dim - heatPattern.getColumnHigherBoundary())
		//					) {
		//					boardIn.at(dim * i + j) = boardOut.at(dim * i + j);
		//					continue;
		//				}
		//				// if not bordering add
		//				float sum = 0;
		//				int factor = 0;
		//				for (int k = 0; k < heatPattern.size(); k++) {
		//					int ri = i + heatPattern.rowOffset(k);
		//					int ci = j + heatPattern.columnOffset(k);
		//					sum += boardOut.at(dim * ri + ci) * heatPattern.itemWeight(k);
		//					factor += heatPattern.itemWeight(k);
		//				}
		//				sum = sum / factor;
		//				boardIn.at(dim * i + j) = sum;
		//			}
		//		}
		//	}
		//	auto end = std::chrono::system_clock::now();
		//	std::chrono::duration<double, std::milli> time = end - start;
		//	std::cout << "Stencil::Sequential::" << std::to_string(time.count()) << std::endl;
		//}

		//printPPM(dim,"Sequential", maxiters, threadcount);


		// TEST PARALLEL
		for (int t = 0; t < thdv_test_count; t++) {
			initPlate(dim);
			auto start = std::chrono::system_clock::now();
			start = std::chrono::system_clock::now();
			for (int iter = 0; iter < maxiters; iter += 2) {
				// Forwards iteration
				stencil(boardOut, boardIn, heatPattern, PSLED_CROP, dim, dim);
				// Backwards iteration
				stencil(boardIn, boardOut, heatPattern, PSLED_CROP, dim, dim);
			}

			auto end = std::chrono::system_clock::now();
			std::chrono::duration<double, std::milli> time = end - start;
	
		std::cout << "Stencil::Parallel::" << std::to_string(time.count()) << std::endl;
		}
	}


}
#endif // !_TEST_HEAT_DISTRIBUTION_VECTOR_HPP
