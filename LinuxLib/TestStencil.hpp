#ifndef TEST_STENCIL_HPP
#define TEST_STENCIL_HPP
#include <iostream>
#include <string>
#include <vector>
#include <utility>
#include "Stencil.hpp"

namespace ts {

#define tsXDSIZE 4
#define tsYDSIZE 5
#define tsTHREADCOUNT 4

	int curr_tests = 0;
	int pass_tests = 0;
	Pattern pattern;
	float in[tsXDSIZE][tsYDSIZE];
	float pout[tsXDSIZE][tsYDSIZE];
	float sout[tsXDSIZE][tsYDSIZE];
	auto stencil = Stencil(tsTHREADCOUNT);

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
		for (int i = 0; i < tsXDSIZE; i++) {
			for (int j = 0; j < tsYDSIZE; j++)
				in[i][j] = RandomFloat(0.0f, 15.0f);
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
		for (int i = 0; i < tsXDSIZE; i++) {
			for (int j = 0; j < tsYDSIZE; j++) {
				pout[i][j] = 0;
				sout[i][j] = 0;
			}
		}
	}

	// Test running and checking
	// ----------------------------------------------------------
	void sequentialNormalTest() {
		for (int i = 0; i < tsXDSIZE; i++) {
			for (int j = 0; j < tsYDSIZE; j++) {
				float sum = 0;
				int c = 0;
				for (int k = 0; k < pattern.size(); k++) {
					int ro = i + pattern.rowOffset(k);
					int co = j + pattern.columnOffset(k);
					if (ro < 0 || ro >= tsXDSIZE || co < 0 || co >=tsYDSIZE) continue;
					c += pattern.itemWeight(k);

					sum += in[ro][co];
				}
				sum = sum / c;
				sout[i][j] = sum;
			}
		}
	}

	void sequentialWrapTest() {
		for (int i = 0; i < tsXDSIZE; i++) {
			for (int j = 0; j < tsYDSIZE; j++) {
				float sum = 0;
				int c = 0;
				for (int k = 0; k < pattern.size(); k++) {
					int ro = ((i + pattern.rowOffset(k) + tsXDSIZE) % tsXDSIZE);
					int co = ((j + pattern.columnOffset(k) + tsYDSIZE) % tsYDSIZE);
					c += pattern.itemWeight(k);

					sum += in[ro][co];
				}
				sum = sum / c;
				sout[i][j] = sum;
			}
		}
	}

	void sequentialBorderTest() {
		for (int i = 0; i < tsXDSIZE; i++) {
			for (int j = 0; j < tsYDSIZE; j++) {
				if (i < -pattern.getRowLowerBoundary()
					|| i >= (tsXDSIZE - pattern.getRowHigherBoundary())
					|| j < -pattern.getColumnLowerBoundary()
					|| j >= (tsYDSIZE - pattern.getColumnHigherBoundary())) 
					continue;

				float sum = 0;
				int c = 0;
				for (int k = 0; k < pattern.size(); k++) {
					int ro = i + pattern.rowOffset(k);
					int co = j + pattern.columnOffset(k);
					c += pattern.itemWeight(k);

					sum += in[ro][co];
				}
				sum = sum / c;

				sout[i][j] = sum;
			}
		}
	}

	void sequentialMirrorTest() {
		for (int i = 0; i < tsXDSIZE; i++) {
			for (int j = 0; j < tsYDSIZE; j++) {
				float sum = 0;
				int c = 0;
				for (int k = 0; k < pattern.size(); k++) {
					int ro = pattern.rowOffset(k) + i;
					if (ro < 0 || ro >= tsXDSIZE) ro = i - pattern.rowOffset(k);
					int co = pattern.columnOffset(k) + j;
					if (co < 0 || co >= tsYDSIZE) co = j - pattern.columnOffset(k);
					c += pattern.itemWeight(k);
					
					sum += in[ro][co];
				}
				sum = sum / c;
				sout[i][j] = sum;
			}
		}
	}

	void equivalenceCheck() {
		bool flag = false;
		curr_tests++;
		std::cout << "\nTest outcome: ";
		for (int i = 0; i < tsXDSIZE; i++) {
			for (int j = 0; j < tsYDSIZE; j++) {
				if (pout[i][j] != sout[i][j]) {
					flag = true;
					std::cout << "WRONG OUTPUT \n -> at (" << i << ", " << j << ")\n";
					std::cout << "Sequential output = " << sout[i][j] << "\n";
					std::cout << "Parallel output   = " << pout[i][j] << "\n";
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
		for (int i = 0; i < tsXDSIZE; i++) {
			for (int j = 0; j < tsYDSIZE; j++) {
				std::cout << in[i][j] << " ";
			}
			std::cout << "\n";
		}
	}

	void printParallelOutput() {
		std::cout << "Parallel output: \n";
		for (int i = 0; i < tsXDSIZE; i++) {
			for (int j = 0; j < tsYDSIZE; j++) {
				std::cout << pout[i][j] << " ";
			}
			std::cout << "\n";
		}

	}

	void printSeqentialOutput() {
		std::cout << "Sequential output: \n";
		// print sout
		for (int i = 0; i < tsXDSIZE; i++) {
			for (int j = 0; j < tsYDSIZE; j++) {
				std::cout << sout[i][j] << " ";
			}
			std::cout << "\n";
		}

	}

	// Testing
	// -----------------------------------------------------------
	void allTests() {
		// TEST: NORMAL
		// ----------------------------------------------------------
		std::cout << "Sequential: NORMAL \n";
		sequentialNormalTest();
		std::cout << "Parallel:   NORMAL \n";
		stencil(pout, in, pattern, PSLED_NORMAL);
		equivalenceCheck();
		//	printInput();
		//	printSeqentialOutput();
		//	printParallelOutput();
		std::cout << "\n###########################################\n";
		clearOutputs();

		// TEST: WRAP
		// ----------------------------------------------------------
		std::cout << "Sequential: WRAP \n";
		sequentialWrapTest();
		std::cout << "Parallel:   WRAP \n";
		stencil(pout, in, pattern, PSLED_WRAP);
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
		stencil(pout, in, pattern, PSLED_MIRROR);
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
		stencil(pout, in, pattern, PSLED_BORDER);
		equivalenceCheck();
		//	printInput();
		//	printSeqentialOutput();
		//	printParallelOutput();
		std::cout << "\n###########################################\n";
		clearOutputs();
	}

	void test() {
		initInput();


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

#endif // ! TEST_STENCIL_HPP
