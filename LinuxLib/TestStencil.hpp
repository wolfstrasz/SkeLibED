#ifndef TEST_STENCIL_HPP
#define TEST_STENCIL_HPP

#include <chrono>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <utility>
#include "Stencil.hpp"
#include "DynamicMap6.hpp"
#include "Pixel.hpp"
#include "TestPixel.hpp"
//#include "TestImg.hpp"

namespace teststencil {


#define XDSIZE 10
#define YDSIZE 10


	Pattern os;
	float in[XDSIZE][YDSIZE];
	float out[XDSIZE][YDSIZE];
	float sout[XDSIZE][YDSIZE];
	int threadcount = 4;
	auto st = Stencil(threadcount);

	float RandomFloat(float a, float b) {
		float random = ((float)rand()) / (float)RAND_MAX;
		float diff = b - a;
		float r = random * diff;
		return a + r;
	}
	void sequentialNormalTest() {
		std::cout << "Sequential NORMAL run\n";
		// Calculate sequential solutions
		for (int i = 0; i < XDSIZE; i++) {
			for (int j = 0; j < YDSIZE; j++) {
				float sum = 0;
				int c = 0;
				for (int k = 0; k < os.size(); k++) {
					int ro = i + os.rowOffset(k);
					int co = j + os.columnOffset(k);

					if (ro < 0 || ro >= XDSIZE || co < 0 || co >=YDSIZE) continue;
					c++;
					sum += in[ro][co];
					// std::cout << sum << std::endl;

				}
				//	std::cout <<"END SUM: " <<  sum << std::endl;
				sum = sum / c;
				sout[i][j] = sum;
			}
		}
	}

	void sequentialWrapTest() {
		std::cout << "Seuential WRAP run\n";
		// Calculate sequential solutions
		for (int i = 0; i < XDSIZE; i++) {
			for (int j = 0; j < YDSIZE; j++) {
				float sum = 0;
				for (int k = 0; k < os.size(); k++) {

					sum += in[((i + os.rowOffset(k) + XDSIZE) % XDSIZE)][((j + os.columnOffset(k) + YDSIZE) % YDSIZE)];
					// std::cout << sum << std::endl;

				}
				//	std::cout <<"END SUM: " <<  sum << std::endl;
				sum = sum / os.size();
				sout[i][j] = sum;
			}
		}
	}

	void sequentialBorderTest() {
		std::cout << "Sequential BORDER run\n";
		for (int i = 0; i < XDSIZE; i++) {
			for (int j = 0; j < YDSIZE; j++) {
			//	std::cout << i << " " << j << std::endl;
				if (i < -os.getRowLowerBoundary()
					|| i >= (XDSIZE - os.getRowHigherBoundary())
					|| j < -os.getColumnLowerBoundary()
					|| j >= (YDSIZE - os.getColumnHigherBoundary())) {
					sout[i][j] = in[i][j];
				//	std::cout << " BORDER\n";
					continue;
				}
				float sum = 0;
				for (int k = 0; k < os.size(); k++) {
				//	std::cout << "SUM : " << sum << "\n";
				//	std::cout << "added to sum :" << in[i + os.rowOffset(k)][j + os.columnOffset(k)]  << "\n";
					sum += in[i + os.rowOffset(k)][j + os.columnOffset(k)];
					// std::cout << sum << std::endl;

				}
				//	std::cout <<"END SUM: " <<  sum << std::endl;
				sum = sum / os.size();
				sout[i][j] = sum;
				//std::cout << "SUM : " << sout[i][j] << "\n";
			}
		}
	}

	void sequentialMirrorTest() {
		std::cout << "Seuential Mirror run\n";
		// Calculate sequential solution
		for (int i = 0; i < XDSIZE; i++) {
			for (int j = 0; j < YDSIZE; j++) {
				float sum = 0;
				for (int k = 0; k < os.size(); k++) {
					int ro = os.rowOffset(k) + i;
					if (ro < 0 || ro >= XDSIZE) ro = i - os.rowOffset(k);

					int co = os.columnOffset(k) + j;
					if (co < 0 || co >= YDSIZE) co = j - os.columnOffset(k);

					sum += in[ro][co];
				//	std::cout << "adding : " << in[ro][co] << " sum = " << sum <<"\n";
				}
			//	std::cout <<"END SUM: " <<  sum << std::endl;
				sum = sum / os.size();
				sout[i][j] = sum;
			}
		}
	}

	void printParallelOutput() {

		std::cout << "Parallel output: \n";
		for (int i = 0; i < XDSIZE; i++) {
			for (int j = 0; j < YDSIZE; j++) {
				std::cout << out[i][j] << " ";
			}
			std::cout << "\n";
		}

	}

	void printSeqentialOutput() {

		std::cout << "Sequential OUT : \n";
		// print sout
		for (int i = 0; i < XDSIZE; i++) {
			for (int j = 0; j < YDSIZE; j++) {
				std::cout << sout[i][j] << " ";
			}
			std::cout << "\n";
		}

	}

	void printInput() {
		std::cout << "Print INPUT:\n";
		for (int i = 0; i < XDSIZE; i++) {
			for (int j = 0; j < YDSIZE; j++) {
				std::cout << in[i][j] << " ";
			}
			std::cout << "\n";
		}
	}

	void equivalenceCheck() {
		bool flag = false;
		for (int i = 0; i < XDSIZE; i++) {
			for (int j = 0; j < YDSIZE; j++) {
				std::cout << out[i][j] << " ";
				if (out[i][j] != sout[i][j]) {
					flag = true;
					std::cout << "WRONG OUTPUT\n";
					std::cout <<"Seq = " << sout[i][j] << " --> ";
					std::cout <<"Par = " << out[i][j] << " ";
					goto endloops;
				}
			}
			std::cout << "\n";
		}
	endloops:
		if (!flag) std::cout << "\nSAME OUTPUT\n";
	}

	void clearOutputs() {
		for (int i = 0; i < XDSIZE; i++) {
			for (int j = 0; j < YDSIZE; j++) {
				out[i][j] = 0;
				sout[i][j] = 0;
			}
		}
	}


	void test() {

		// testImgWrite();
		// testImgRead();

		//	std::cout << "INIT 2D array\n";
			// Initialise input 2D-array
		for (int i = 0; i < XDSIZE; i++) {
			for (int j = 0; j < YDSIZE; j++)
				//in[i][j] = i * YDSIZE + j;
				in[i][j] = RandomFloat(0.0f, 15.0f);
		}


		// Initialise offsets
		os.add(0, 0);
		os.add(-1, 0);
		os.add(0, 1);
		os.add(1, 0);
		os.add(0, -1);


		std::cout << "##############################################################################\n";
		sequentialWrapTest();
		//printInput();
		//printSeqentialOutput();
		std::cout << "Parallel WRAP run \n";
		st(out, in, os, PSLED_WRAP);
		equivalenceCheck();
		clearOutputs();
		//printSeqentialOutput();

		std::cout << "##############################################################################\n";
		sequentialBorderTest();
	//	printInput();
	//	printSeqentialOutput();
		std::cout << "Parallel BORDER run \n";
		st(out, in, os, PSLED_BORDER);
	//	printParallelOutput();
		equivalenceCheck();
		clearOutputs();

		std::cout << "##############################################################################\n";
	//	printInput();
		sequentialMirrorTest();
	//	printSeqentialOutput();
		std::cout << "Parallel MIRROR run \n";
		st(out, in, os, PSLED_MIRROR);
	//	printParallelOutput();
		equivalenceCheck();
		clearOutputs();

		std::cout << "##############################################################################\n";
		sequentialNormalTest();
	//	printInput();
	//	printSeqentialOutput();
		std::cout << "Parallel NORMAL run \n";
		st(out, in, os, PSLED_NORMAL);
	//	printParallelOutput();
		equivalenceCheck();
		clearOutputs();

		
	}
}

#endif // ! TEST_STENCIL_HPP
