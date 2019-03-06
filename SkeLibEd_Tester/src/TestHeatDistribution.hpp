#ifndef _TEST_HEAT_DISTRIBUTION
#define _TEST_HEAT_DISTRIBUTION

#include "Pixel.hpp"
#include "Stencil.hpp"
#include <chrono>
#include <iostream>
#include <fstream>
#include <string>
#include <math.h>
/* uses Jacobi method to solve a sparse system of linear equations */
/* to solve we need conditions at the border to be kept constant */
#define MAX_PLATE_DIM 1024
float boardIn[MAX_PLATE_DIM][MAX_PLATE_DIM];			// iterative simulation would require us to switch betweeen Input and Output
float boardOut[MAX_PLATE_DIM][MAX_PLATE_DIM];
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
	for (int i = 0; i < MAX_PLATE_DIM ; i++) {
	//	std::cout << i << std::endl;
		for (int j = 0; j < MAX_PLATE_DIM; j++) {
		//	std::cout << j << std::endl;
			boardIn[i][j] = 1.0f;
			boardOut[i][j] = 1.0f;
		}
	}
	// Initialise hot spot (from below and top the plate) with red value (3.0f);
	for (int j = 1; j < MAX_PLATE_DIM - 1; j++) {
		boardIn[MAX_PLATE_DIM - 1][j] = 3.0f;
		boardIn[0][j] = 3.0f;

		boardOut[MAX_PLATE_DIM - 1][j] = 3.0f;
		boardOut[0][j] = 3.0f;
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
	std::string outfileName = "heat_" + name + "_" + std::to_string(threads) + "_" + std::to_string(iters) + ".ppm";
	FILE *outfile;
	outfile = fopen(outfileName.c_str(), "w");
	fprintf(outfile, "P6\n");
	fprintf(outfile, "%d %d\n%d\n", MAX_PLATE_DIM, MAX_PLATE_DIM, 255);

	psled::pPixel heatPixel;
	for (int i = 0; i < MAX_PLATE_DIM; i++) {
		for (int j = 0; j < MAX_PLATE_DIM; j++) {
			heatPixel = convertToHeatMap(boardIn[i][j]);
			fputc((char)heatPixel.r, outfile);
			fputc((char)heatPixel.g, outfile);
			fputc((char)heatPixel.b, outfile);
		}
	}

	fclose(outfile);
}
/* https://www.sciencedirect.com/topics/computer-science/stencil-pattern */
/* https://www.cs.uky.edu/~jzhang/CS621/chapter7.pdf */
void testHeatDistribution(int maxiters = 0, int threadcount = 0 ) {
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
			for (int i = 0; i < MAX_PLATE_DIM; i++) {
				for (int j = 0; j < MAX_PLATE_DIM; j++) {
					// Check for border
					if (i < -heatPattern.getRowLowerBoundary()
						|| i >= (MAX_PLATE_DIM - heatPattern.getRowHigherBoundary())
						|| j < -heatPattern.getColumnLowerBoundary()
						|| j >= (MAX_PLATE_DIM - heatPattern.getColumnHigherBoundary())
						) {
						boardOut[i][j] = boardIn[i][j];
						continue;
					}
					// if not bordering add
					float sum = 0;
					for (int k = 0; k < heatPattern.size(); k++) {
						sum += boardIn[i + heatPattern.rowOffset(k)][j + heatPattern.columnOffset(k)];

					}
					sum = sum / heatPattern.size();
					boardOut[i][j] = sum;
				}
			}
		// Backwards iteration OUT --> IN
			for (int i = 0; i < MAX_PLATE_DIM; i++) {
				for (int j = 0; j < MAX_PLATE_DIM; j++) {
					// Check for border
					if (i < -heatPattern.getRowLowerBoundary()
						|| i >= (MAX_PLATE_DIM - heatPattern.getRowHigherBoundary())
						|| j < -heatPattern.getColumnLowerBoundary()
						|| j >= (MAX_PLATE_DIM - heatPattern.getColumnHigherBoundary())
						) {
						boardIn[i][j] = boardOut[i][j];
						continue;
					}
					// if not bordering add
					float sum = 0;
					for (int k = 0; k < heatPattern.size(); k++) {
						sum += boardOut[i + heatPattern.rowOffset(k)][j + heatPattern.columnOffset(k)];

					}
					sum = sum / heatPattern.size();
					boardIn[i][j] = sum;
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
		stencil(boardOut, boardIn, heatPattern, PSLED_BORDER);
		// Backwards iteration
		stencil(boardIn, boardOut, heatPattern, PSLED_BORDER);
	}
	end = std::chrono::system_clock::now();
	printPPM("Parallel", maxiters, threadcount);

	time = end - start;
	std::cout << "Stencil::Parallel::" << std::to_string(time.count()) << std::endl;


}



#endif // !_TEST_HEATH_DISTRIBUTION
