#ifndef _TEST_STENCIL_VECTOR_HPP
#define _TEST_STENCIL_VECTOR_HPP
#include <iostream>
#include <string>
#include <vector>
#include <utility>
#include "Stencil.hpp"

namespace tsv {

#define tsvYDSIZE 10
#define tsvXDSIZE 10
#define tsvTHREADCOUNT 4

	int curr_tests = 0;
	int pass_tests = 0;
	Pattern pattern;
	std::vector<float> in;
	std::vector<float> sout;
	std::vector<float> pout;
	auto stencil = Stencil(tsvTHREADCOUNT);

	// Random float number generator
	// ----------------------------------------------------------
	float RandomFloat(float a, float b) {
		float random = ((float)rand()) / (float)RAND_MAX;
		float diff = b - a;
		float r = random * diff;
		return a + r;
	}

	// Pre-test init and tidy-up
	// ----------------------------------------------------------
	void initInput() {
		for (int i = 0; i < tsvXDSIZE; i++) {
			for (int j = 0; j < tsvYDSIZE; j++)
				in.push_back(RandomFloat(0.0f, 15.0f));
		}
	}

	void initPattern() {
		// Initialise offsets
		pattern.add(0, 0);
		pattern.add(-1, 0);
		pattern.add(0, 1);
		pattern.add(1, 0);
		pattern.add(0, -1);
		// at this point it is a cross
	}

	void extendToSquarePattern() {
		pattern.add(-1, -1);
		pattern.add(-1, 1);
		pattern.add(1, 1);
		pattern.add(1, -1);
		// at this point it is a square
	}

	void extendToDiamondPattern() {
		pattern.add(-2, 0);
		pattern.add(0, 2);
		pattern.add(2, 0);
		pattern.add(0, -2);
		// at this point it is a diamond-shaped
	}
	void clearOutputs() {
		for (int i = 0; i < tsvXDSIZE; i++) {
			for (int j = 0; j < tsvYDSIZE; j++) {
				pout.at(i*tsvYDSIZE + j) = 0;
				sout.at(i*tsvYDSIZE + j) = 0;
			}
		}
	}

	// Test running and checking
	// ----------------------------------------------------------
	void sequentialNormalTest() {
		for (int i = 0; i < tsvXDSIZE; i++) {
			for (int j = 0; j < tsvYDSIZE; j++) {
				float sum = 0;
				int c = 0;
				for (int k = 0; k < pattern.size(); k++) {
					int ro = i + pattern.rowOffset(k);
					int co = j + pattern.columnOffset(k);
					if (ro < 0 || ro >= tsvXDSIZE || co < 0 || co >= tsvYDSIZE)
						continue;
					c += pattern.itemWeight(k);
					sum += in.at(ro * tsvYDSIZE + co);
				}
				sum = sum / c;
				sout.at(i* tsvYDSIZE + j) = sum;
			}
		}
	}

	void sequentialWrapTest() {
		for (int i = 0; i < tsvXDSIZE; i++) {
			for (int j = 0; j < tsvYDSIZE; j++) {
				float sum = 0;
				int c = 0;
				for (int k = 0; k < pattern.size(); k++) {
					int ro = ((i + pattern.rowOffset(k) + tsvXDSIZE) % tsvXDSIZE);
					int co = ((j + pattern.columnOffset(k) + tsvYDSIZE) % tsvYDSIZE);
					c += pattern.itemWeight(k);
					sum += in.at(ro * tsvYDSIZE + co);
				}
				sum = sum / c;
				sout.at(i* tsvYDSIZE + j) = sum;
			}
		}
	}

	void sequentialBorderTest() {
		for (int i = 0; i < tsvXDSIZE; i++) {
			for (int j = 0; j < tsvYDSIZE; j++) {
				if (i < -pattern.getRowLowerBoundary()
					|| i >= (tsvXDSIZE - pattern.getRowHigherBoundary())
					|| j < -pattern.getColumnLowerBoundary()
					|| j >= (tsvYDSIZE - pattern.getColumnHigherBoundary()))
					continue;

				float sum = 0;
				int c = 0;
				for (int k = 0; k < pattern.size(); k++) {
					int ro = i + pattern.rowOffset(k);
					int co = j + pattern.columnOffset(k);
					c += pattern.itemWeight(k);
					sum += in.at(ro * tsvYDSIZE + co);
				}
				sum = sum / c;
				sout.at(i* tsvYDSIZE + j) = sum;
			}
		}
	}

	void sequentialMirrorTest() {
		for (int i = 0; i < tsvXDSIZE; i++) {
			for (int j = 0; j < tsvYDSIZE; j++) {
				float sum = 0;
				int c = 0;
				for (int k = 0; k < pattern.size(); k++) {
					int ro = pattern.rowOffset(k) + i;
					if (ro < 0 || ro >= tsvXDSIZE) ro = i - pattern.rowOffset(k);
					int co = pattern.columnOffset(k) + j;
					if (co < 0 || co >= tsvYDSIZE) co = j - pattern.columnOffset(k);
					c += pattern.itemWeight(k);
					sum += in.at(ro * tsvYDSIZE + co);
				}
				sum = sum / c;
				sout.at(i* tsvYDSIZE + j) = sum;
			}
		}
	}

	void equivalenceCheck() {
		bool flag = false;
		curr_tests++;
		std::cout << "\nTest outcome: ";
		for (int i = 0; i < tsvXDSIZE; i++) {
			for (int j = 0; j < tsvYDSIZE; j++) {
				if (pout.at(i* tsvYDSIZE + j) != sout.at(i* tsvYDSIZE + j)) {
					flag = true;
					std::cout << "WRONG OUTPUT \n -> at (" << i << ", " << j << ")\n";
					std::cout << "Sequential output = " << sout.at(i* tsvYDSIZE + j) << "\n";
					std::cout << "Parallel output   = " << pout.at(i* tsvYDSIZE + j) << "\n";
					goto endloops;
				}
			}
		}
	endloops:
		if (!flag) {
			std::cout << "PASSED\n";
			pass_tests++;
		}
	}

	// Printing
	// ----------------------------------------------------------
	void printInput() {
		std::cout << "Print input:\n";
		for (int i = 0; i < tsvXDSIZE; i++) {
			for (int j = 0; j < tsvYDSIZE; j++) {
				std::cout << in.at(i* tsvYDSIZE + j) << " ";
			}
			std::cout << "\n";
		}
	}

	void printParallelOutput() {
		std::cout << "Parallel output: \n";
		for (int i = 0; i < tsvXDSIZE; i++) {
			for (int j = 0; j < tsvYDSIZE; j++) {
				std::cout << pout.at(i* tsvYDSIZE + j) << " ";
			}
			std::cout << "\n";
		}

	}

	void printSeqentialOutput() {
		std::cout << "Sequential output: \n";
		// print sout
		for (int i = 0; i < tsvXDSIZE; i++) {
			for (int j = 0; j < tsvYDSIZE; j++) {
				std::cout << sout.at(i* tsvYDSIZE + j) << " ";
			}
			std::cout << "\n";
		}

	}

	// Testing
	// -----------------------------------------------------------
	void allTests() {
	
		// TEST: WRAP
		// ----------------------------------------------------------
		std::cout << "Sequential: WRAP \n";
		sequentialWrapTest();
		std::cout << "Parallel:   WRAP \n";
		stencil(pout, in, pattern, PSLED_WRAP, tsvXDSIZE, tsvYDSIZE);
		equivalenceCheck();
		//	printInput();
		//	printSeqentialOutput();
		//	printParallelOutput();
		std::cout << "\n###########################################\n";
		clearOutputs();

		// TEST: MIRROR
		// ----------------------------------------------------------
		std::cout << "Sequential: MIRROR \n";
		sequentialMirrorTest();
		std::cout << "Parallel:   MIRROR\n";
		stencil(pout, in, pattern, PSLED_MIRROR, tsvXDSIZE, tsvYDSIZE);
		equivalenceCheck();
		//	printInput();
		//	printSeqentialOutput();
		//	printParallelOutput();
		std::cout << "\n###########################################\n";
		clearOutputs();

		// TEST: BORDER
		// ----------------------------------------------------------
		std::cout << "Sequential: BORDER \n";
		sequentialBorderTest();
		std::cout << "Parallel:   BORDER\n";
		stencil(pout, in, pattern, PSLED_BORDER, tsvXDSIZE, tsvYDSIZE);
		equivalenceCheck();
		//	printInput();
		//	printSeqentialOutput();
		//	printParallelOutput();
		std::cout << "\n###########################################\n";
		clearOutputs();

		// TEST: NORMAL
		// ----------------------------------------------------------
		std::cout << "Sequential: NORMAL \n";
		sequentialNormalTest();
		std::cout << "Parallel:   NORMAL \n";
		stencil(pout, in, pattern, PSLED_NORMAL, tsvXDSIZE, tsvYDSIZE);
		equivalenceCheck();
		// printInput();
		// printSeqentialOutput();
		// printParallelOutput();
		std::cout << "\n###########################################\n";
		clearOutputs();

		// TEST: NORMAL OPT 1
		// ----------------------------------------------------------
		std::cout << "Sequential: NORMAL \n";
		sequentialNormalTest();
		std::cout << "Parallel:   NORMAL OPT 1\n";
		stencil(pout, in, pattern, PSLED_NORMAL_OPT1, tsvXDSIZE, tsvYDSIZE);
		equivalenceCheck();
		//	printInput();
		//	printSeqentialOutput();
		//	printParallelOutput();
		std::cout << "\n###########################################\n";
		clearOutputs();

		// TEST: NORMAL OPT 2
		// ----------------------------------------------------------
		std::cout << "Sequential: NORMAL \n";
		sequentialNormalTest();
		std::cout << "Parallel:   NORMAL OPT 2\n";
		stencil(pout, in, pattern, PSLED_NORMAL_OPT2, tsvXDSIZE, tsvYDSIZE);
		equivalenceCheck();
		//printInput();
		//printSeqentialOutput();
		//printParallelOutput();
		std::cout << "\n###########################################\n";
		clearOutputs();

		// TEST: NORMAL OPT 3
		// ----------------------------------------------------------
		std::cout << "Sequential: NORMAL \n";
		sequentialNormalTest();
		std::cout << "Parallel:   NORMAL OPT 3\n";
		stencil(pout, in, pattern, PSLED_NORMAL_OPT3, tsvXDSIZE, tsvYDSIZE);
		equivalenceCheck();
	//	printInput();
	//	printSeqentialOutput();
	//	printParallelOutput();
		std::cout << "\n###########################################\n";
		clearOutputs();

		// TEST: NORMAL OPT 4
		// ----------------------------------------------------------
		std::cout << "Sequential: NORMAL \n";
		sequentialNormalTest();
		std::cout << "Parallel:   NORMAL OPT 4\n";
		stencil(pout, in, pattern, PSLED_NORMAL_OPT4, tsvXDSIZE, tsvYDSIZE);
		equivalenceCheck();
		printInput();
		printSeqentialOutput();
		printParallelOutput();
		std::cout << "\n###########################################\n";
		clearOutputs();
	}

	void test() {

		initInput();
		pout = std::vector<float>(in.size());
		sout = std::vector<float>(in.size());
		std::cout << "\n\n\nTest stencil patttern: CROSS\n\n";
		initPattern();
		allTests();

		std::cout << "\n\n\nTest stencil pattern: SQUARE\n\n";
		extendToSquarePattern();
		allTests();

		std::cout << "\n\n\nTest stencil pattern: DIAMOND\n\n";
		extendToDiamondPattern();
		allTests();


		std::cout << "*******************************************\n";
		std::cout << "TESTS PASSED:    " << pass_tests << "  /  " << curr_tests << "\n";
	}
}
#endif // !_TEST_STENCIL_VECTOR_HPP
