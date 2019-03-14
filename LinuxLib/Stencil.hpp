#ifndef STENCIL_HPP
#define STENCIL_HPP
#include <cstdlib>
#include <iostream>
#include <vector>
#include <type_traits>
#include <functional>
#include <stdarg.h>
#include <typeinfo>
#include <utility>
#include <thread>
#include <mutex>
#include "Pattern.hpp"

//namespace psled {
#define PSLED_WRAP StencilSkeleton::BoundaryHandling::WRAP
#define PSLED_NORMAL StencilSkeleton::BoundaryHandling::NORMAL
#define PSLED_MIRROR StencilSkeleton::BoundaryHandling::MIRROR
#define PSLED_BORDER StencilSkeleton::BoundaryHandling::BORDER
#define PSLED_NORMAL_OPT1 StencilSkeleton::BoundaryHandling::NORMAL_OPT_1
#define PSLED_NORMAL_OPT2 StencilSkeleton::BoundaryHandling::NORMAL_OPT_2
#define PSLED_NORMAL_OPT3 StencilSkeleton::BoundaryHandling::NORMAL_OPT_3
#define PSLED_NORMAL_OPT4 StencilSkeleton::BoundaryHandling::NORMAL_OPT_4

class StencilSkeleton {

private:
	StencilSkeleton() {}

public:
	/*
		* Enumerator class used to choose what kind of boundary handling the Stencil should use.
		*/
	enum class BoundaryHandling {
		NORMAL,
		WRAP,
		MIRROR,
		BORDER,
		NORMAL_OPT_1,
		NORMAL_OPT_2,
		NORMAL_OPT_3,
		NORMAL_OPT_4,

	};


	// StencilImplementation
	// ---------------------
	class StencilImplementation {
	private:
		int no_factor = 1;					// used when there should be no normalization (read-only)
		std::vector<std::thread *> THREADS;
		size_t nthreads;
		size_t num_heavier_threads;
		size_t rows_per_thread;
		size_t index_begin;
		size_t index_end;
		size_t t;
		template <typename T>
		class Scoreboard {
		public:
			Pattern pattern;
			int rows, cols;
			T* input;
			T* output;
			Scoreboard(Pattern &pattern, T *input, T *output, int rows = 0, int cols = 0) {
				this->pattern = pattern;
				this->input = input;
				this->output = output;
				this->rows = rows;
				this->cols = cols;
			}
		};
		void * scoreboard;

		// Functionality: Wrap - Uses wrapping around boundaries
		// -----------------------------------------------------
		template<typename T>
		void threadWrap(Scoreboard<T>* scoreboard, size_t startIndex, size_t endIndex) {
			Pattern &pattern = scoreboard->pattern;
			T sum;
			sum -= sum;
			int factor;
			int* factor_ptr = scoreboard->pattern.normalization ? &factor : &no_factor;
			for (int rowIndex = startIndex; rowIndex < endIndex; rowIndex++) {
				for (int colIndex = 0; colIndex < scoreboard->cols; colIndex++) {

					factor = 0;
					for (int offsetIndex = 0; offsetIndex < pattern.size(); offsetIndex++) {
						int i = (pattern.rowOffset(offsetIndex) + rowIndex);
						int j = (pattern.columnOffset(offsetIndex) + colIndex);
						i = (i + scoreboard->rows) % scoreboard->rows;
						j = (j + scoreboard->cols) % scoreboard->cols;
						sum += (*(scoreboard->input + i * scoreboard->cols + j)) * pattern.itemWeight(offsetIndex);
						factor += pattern.itemWeight(offsetIndex);
					}

					sum = sum / *factor_ptr;
					*(scoreboard->output + rowIndex * scoreboard->cols + colIndex) = sum;
					sum -= sum;
				}
			}

		}

		template<typename T>
		void threadVectorWrap(Scoreboard<std::vector<T>>* scoreboard, size_t startIndex, size_t endIndex) {
			//std::cout << __PRETTY_FUNCTION__ << std::endl;
			Pattern &pattern = scoreboard->pattern;
			T sum;
			sum -= sum;
			int xdim = scoreboard->rows;
			int ydim = scoreboard->cols;

			int factor;
			int* factor_ptr = scoreboard->pattern.normalization ? &factor : &no_factor;
			for (int rowIndex = startIndex; rowIndex < endIndex; rowIndex++) {
				for (int colIndex = 0; colIndex < scoreboard->cols; colIndex++) {
					factor = 0;
					for (int offsetIndex = 0; offsetIndex < pattern.size(); offsetIndex++) {
						int i = (pattern.rowOffset(offsetIndex) + rowIndex);
						int j = (pattern.columnOffset(offsetIndex) + colIndex);
						// wrapping
						i = (i + xdim) % xdim; // addition required because -4 % 5 is not = 1 by definition of % operator
						j = (j + ydim) % ydim;
						sum += scoreboard->input->at(i*ydim + j);
						factor += pattern.itemWeight(offsetIndex);
					}
					sum = sum / *factor_ptr;
					scoreboard->output->at(rowIndex * ydim + colIndex) = sum;
					sum -= sum;
				}
			}
		}


		// ThreadMirror - function applied to each thread
		// Functionality: Mirror - Uses mirrored values when handling boundaries
		// ----------------------------------------------------------------------
		template<typename T>
		void threadMirror(Scoreboard<T>* scoreboard, size_t startIndex, size_t endIndex) {
			Pattern &pattern = scoreboard->pattern;
			T sum;
			sum -= sum;
			int factor;
			int* factor_ptr = scoreboard->pattern.normalization ? &factor : &no_factor;
			for (int rowIndex = startIndex; rowIndex < endIndex; rowIndex++) {
				for (int colIndex = 0; colIndex < scoreboard->cols; colIndex++) {

					factor = 0;
					for (int offsetIndex = 0; offsetIndex < pattern.size(); offsetIndex++) {
						int i = (pattern.rowOffset(offsetIndex) + rowIndex);
						int j = (pattern.columnOffset(offsetIndex) + colIndex);
						if (i < 0 || i >= scoreboard->rows) i = (rowIndex - pattern.rowOffset(offsetIndex));
						if (j < 0 || j >= scoreboard->cols) j = (colIndex - pattern.columnOffset(offsetIndex));
						sum += (*(scoreboard->input + i * scoreboard->cols + j)) * pattern.itemWeight(offsetIndex);
						factor += pattern.itemWeight(offsetIndex);
					}

					sum = sum / *factor_ptr;
					*(scoreboard->output + rowIndex * scoreboard->cols + colIndex) = sum;
					sum -= sum;
				}
			}
		}

		template<typename T>
		void threadVectorMirror(Scoreboard<std::vector<T>>* scoreboard, size_t startIndex, size_t endIndex) {
			//std::cout << __PRETTY_FUNCTION__ << std::endl;
			Pattern &pattern = scoreboard->pattern;
			T sum;
			sum -= sum;
			int xdim = scoreboard->rows;
			int ydim = scoreboard->cols;

			int factor;
			int* factor_ptr = scoreboard->pattern.normalization ? &factor : &no_factor;
			for (int rowIndex = startIndex; rowIndex < endIndex; rowIndex++) {
				for (int colIndex = 0; colIndex < scoreboard->cols; colIndex++) {
					factor = 0;
					for (int offsetIndex = 0; offsetIndex < pattern.size(); offsetIndex++) {
						int i = (pattern.rowOffset(offsetIndex) + rowIndex);
						int j = (pattern.columnOffset(offsetIndex) + colIndex);
						// mirroring
						if (i < 0 || i >= xdim) i = (rowIndex - pattern.rowOffset(offsetIndex));
						if (j < 0 || j >= ydim) j = (colIndex - pattern.columnOffset(offsetIndex));

						sum += scoreboard->input->at(i*ydim + j);
						factor += pattern.itemWeight(offsetIndex);
					}
					sum = sum / *factor_ptr;
					scoreboard->output->at(rowIndex * ydim + colIndex) = sum;
					sum -= sum;
				}
			}
		}


		// Functionality: Normal - Does not use other values to exchange unobtainable offset items (cut-off)
		// -------------------------------------------------------------------------------------------------
		template<typename T>
		void threadNormal(Scoreboard<T>* scoreboard, size_t startIndex, size_t endIndex) {
			Pattern &pattern = scoreboard->pattern;
			T sum;
			sum -= sum;
			int factor;
			int* factor_ptr = scoreboard->pattern.normalization ? &factor : &no_factor;
			for (int rowIndex = startIndex; rowIndex < endIndex; rowIndex++) {
				for (int colIndex = 0; colIndex < scoreboard->cols; colIndex++) {
					factor = 0;
					for (int offsetIndex = 0; offsetIndex < pattern.size(); offsetIndex++) {
						int i = (pattern.rowOffset(offsetIndex) + rowIndex);
						int j = (pattern.columnOffset(offsetIndex) + colIndex);
						if (i >= 0 && i < scoreboard->rows && j >= 0 && j < scoreboard->cols) {
							sum += (*(scoreboard->input + i * scoreboard->cols + j)) * pattern.itemWeight(offsetIndex);
							factor += pattern.itemWeight(offsetIndex);

						}
					}

					sum = sum / *factor_ptr;
					*(scoreboard->output + rowIndex * scoreboard->cols + colIndex) = sum;
					sum -= sum;
				}
			}
		}

		template<typename T>
		void threadVectorNormal(Scoreboard<std::vector<T>>* scoreboard, size_t startIndex, size_t endIndex) {
			//std::cout << __PRETTY_FUNCTION__ << std::endl;
			Pattern &pattern = scoreboard->pattern;
			T sum;
			sum = 0;
			int xdim = scoreboard->rows;
			int ydim = scoreboard->cols;

			int factor;
			int* factor_ptr = scoreboard->pattern.normalization ? &factor : &no_factor;
			for (int rowIndex = startIndex; rowIndex < endIndex; rowIndex++) {
				for (int colIndex = 0; colIndex < scoreboard->cols; colIndex++) {
					factor = 0;
				//	std::cout << "\n\n\n =============================================================================================\n";
			//		std::cout << "GOL Main(" << rowIndex << ", " << colIndex << ") : " << scoreboard->input->at(rowIndex * ydim + colIndex) << std::endl;

				//	std::cout << "Zero (" << rowIndex << ", " << colIndex << ") : " << sum << std::endl;
					for (int offsetIndex = 0; offsetIndex < pattern.size(); offsetIndex++) {
			//			std::cout << "\n";

						int i = (pattern.rowOffset(offsetIndex) + rowIndex);
						int j = (pattern.columnOffset(offsetIndex) + colIndex);

						// cut-off check
						if (i >= 0 && i < scoreboard->rows && j >= 0 && j < scoreboard->cols) {
				//			std::cout << "OS (" << pattern.rowOffset(offsetIndex) << ", " << pattern.columnOffset(offsetIndex) << ")  ";
				//			std::cout << "GoL (" << i << ", " << j << ") : " << scoreboard->input->at(i * ydim + j) << std::endl;
							sum += scoreboard->input->at(i*ydim + j);
				//			std::cout << "Sum (" << rowIndex << ", " << colIndex << ") : " << sum << std::endl;
							factor += pattern.itemWeight(offsetIndex);
						}
					}
					sum = sum / *factor_ptr;
				//	std::cout << "Sum Out(" << rowIndex << ", " << colIndex << ") : " << sum << std::endl;
					scoreboard->output->at(rowIndex * ydim + colIndex) = sum;
					sum -= sum;
				}
			}
		}

		template<typename T>
		void threadVectorNormalOpt1(Scoreboard<std::vector<T>>* scoreboard, size_t startIndex, size_t endIndex) {
			Pattern &pattern = scoreboard->pattern;
			T sum;
			sum = 0;
			int xdim = scoreboard->rows;
			int ydim = scoreboard->cols;
			int colLowBoundary = -scoreboard->pattern.getColumnLowerBoundary();
			int colHighBoundary = scoreboard->cols - scoreboard->pattern.getColumnHigherBoundary();
			int rowLowBoundary = -scoreboard->pattern.getRowLowerBoundary();
			int rowHighBoundary = scoreboard->rows - scoreboard->pattern.getRowHigherBoundary();

			int factor;
			int* factor_ptr = scoreboard->pattern.normalization ? &factor : &no_factor;
			int rowIndex = startIndex;

			// RUN IF IN ROW BOUNDARY
			for (; rowIndex < endIndex && rowIndex < rowLowBoundary; rowIndex++) {
				for (int colIndex = 0; colIndex < ydim; colIndex++) {
					sum = 0;
					factor = 0;
					for (int offsetIndex = 0; offsetIndex < pattern.size(); offsetIndex++) {
						//std::cout << "\n";

						int i = (pattern.rowOffset(offsetIndex) + rowIndex);
						int j = (pattern.columnOffset(offsetIndex) + colIndex);

						// cut-off check
						if (i >= 0 && i < scoreboard->rows && j >= 0 && j < scoreboard->cols) {
							sum += scoreboard->input->at(i*ydim + j);
							factor += pattern.itemWeight(offsetIndex);
						}
					}
					sum = sum / *factor_ptr;
					scoreboard->output->at(rowIndex * ydim + colIndex) = sum;
					sum -= sum;
				}
			}

			// RUN IF BETWEENBOUNDARIES
			for (; rowIndex < endIndex && rowIndex < rowHighBoundary; rowIndex++) {
				for (int colIndex = 0; colIndex < colLowBoundary ; colIndex++) {
					factor = 0;
					sum = 0;
					for (int offsetIndex = 0; offsetIndex < pattern.size(); offsetIndex++) {
						//std::cout << "\n";

						int i = (pattern.rowOffset(offsetIndex) + rowIndex);
						int j = (pattern.columnOffset(offsetIndex) + colIndex);

						// cut-off check
						if (i >= 0 && i < scoreboard->rows && j >= 0 && j < scoreboard->cols) {
							sum += scoreboard->input->at(i*ydim + j);
							factor += pattern.itemWeight(offsetIndex);
						}
					}
					sum = sum / *factor_ptr;
					scoreboard->output->at(rowIndex * ydim + colIndex) = sum;
					sum -= sum;
				}

				for (int colIndex = colHighBoundary; colIndex < scoreboard->cols; colIndex++) {
					factor = 0;
					for (int offsetIndex = 0; offsetIndex < pattern.size(); offsetIndex++) {
						//std::cout << "\n";

						int i = (pattern.rowOffset(offsetIndex) + rowIndex);
						int j = (pattern.columnOffset(offsetIndex) + colIndex);

						// cut-off check
						if (i >= 0 && i < scoreboard->rows && j >= 0 && j < scoreboard->cols) {
							sum += scoreboard->input->at(i*ydim + j);
							factor += pattern.itemWeight(offsetIndex);
						}
					}
					sum = sum / *factor_ptr;
					scoreboard->output->at(rowIndex * ydim + colIndex) = sum;
					sum -= sum;
				}
			}


			// Run In top boundary
			for (; rowIndex < endIndex && rowIndex < xdim; rowIndex++) {
				for (int colIndex = 0; colIndex < ydim; colIndex++) {
					sum = 0;
					factor = 0;
					for (int offsetIndex = 0; offsetIndex < pattern.size(); offsetIndex++) {
						//std::cout << "\n";

						int i = (pattern.rowOffset(offsetIndex) + rowIndex);
						int j = (pattern.columnOffset(offsetIndex) + colIndex);

						// cut-off check
						if (i >= 0 && i < scoreboard->rows && j >= 0 && j < scoreboard->cols) {
							sum += scoreboard->input->at(i*ydim + j);
							factor += pattern.itemWeight(offsetIndex);
						}
					}
					sum = sum / *factor_ptr;
					scoreboard->output->at(rowIndex * ydim + colIndex) = sum;
					sum -= sum;
				}
			}
		}


		template<typename T>
		void threadVectorNormalOpt2(Scoreboard<std::vector<T>>* scoreboard, size_t startIndex, size_t endIndex, size_t startIndexBL, size_t endIndexBL) {

			
			Pattern &pattern = scoreboard->pattern;
			T sum;
			int xdim = scoreboard->rows;
			int ydim = scoreboard->cols;
			int factor;
			int* factor_ptr = scoreboard->pattern.normalization ? &factor : &no_factor;
			int colLowBoundary = -pattern.getColumnLowerBoundary();
			int colHighBoundary = scoreboard->cols - pattern.getColumnHigherBoundary();
			int rowLowBoundary = -pattern.getRowLowerBoundary();
			int rowHighBoundary = scoreboard->rows - pattern.getRowHigherBoundary() ;
			int rowIndex = startIndexBL;

			// RUN ON BORDER MODE
			// -----------------------------
			for (; rowIndex < endIndexBL; rowIndex++) {
				for (int colIndex = colLowBoundary; colIndex < colHighBoundary; colIndex++) {
					sum = 0;
					factor = 0;
					for (int offsetIndex = 0; offsetIndex < pattern.size(); offsetIndex++) {
						int i = (pattern.rowOffset(offsetIndex) + rowIndex);
						int j = (pattern.columnOffset(offsetIndex) + colIndex);
						sum += scoreboard->input->at(i*ydim + j) * pattern.itemWeight(offsetIndex);
						factor += pattern.itemWeight(offsetIndex);
					}
					sum = sum / *factor_ptr;
					scoreboard->output->at(rowIndex * ydim + colIndex) = sum;
				}
			}

			sum = 0;
			factor = 0;
			// RUN ON NORMAL MODE
			// --------------------------------------
			rowIndex = startIndex;
			// RUN IF IN ROW BOUNDARY
			for (; rowIndex < endIndex && rowIndex < rowLowBoundary; rowIndex++) {
				for (int colIndex = 0; colIndex < ydim; colIndex++) {
					sum = 0;
					factor = 0;
					for (int offsetIndex = 0; offsetIndex < pattern.size(); offsetIndex++) {
						//std::cout << "\n";

						int i = (pattern.rowOffset(offsetIndex) + rowIndex);
						int j = (pattern.columnOffset(offsetIndex) + colIndex);

						// cut-off check
						if (i >= 0 && i < scoreboard->rows && j >= 0 && j < scoreboard->cols) {
							sum += scoreboard->input->at(i*ydim + j);
							factor += pattern.itemWeight(offsetIndex);
						}
					}
					sum = sum / *factor_ptr;
					scoreboard->output->at(rowIndex * ydim + colIndex) = sum;
					sum -= sum;
				}
			}

			// RUN IF BETWEENBOUNDARIES
			for (; rowIndex < endIndex && rowIndex < rowHighBoundary; rowIndex++) {
				for (int colIndex = 0; colIndex < colLowBoundary; colIndex++) {
					sum = 0;
					factor = 0;
					for (int offsetIndex = 0; offsetIndex < pattern.size(); offsetIndex++) {
						//std::cout << "\n";

						int i = (pattern.rowOffset(offsetIndex) + rowIndex);
						int j = (pattern.columnOffset(offsetIndex) + colIndex);

						// cut-off check
						if (i >= 0 && i < scoreboard->rows && j >= 0 && j < scoreboard->cols) {
							sum += scoreboard->input->at(i*ydim + j);
							factor += pattern.itemWeight(offsetIndex);
						}
					}
					sum = sum / *factor_ptr;
					scoreboard->output->at(rowIndex * ydim + colIndex) = sum;
					sum -= sum;
				}

				for (int colIndex = colHighBoundary; colIndex < scoreboard->cols; colIndex++) {
					sum = 0;
					factor = 0;
					for (int offsetIndex = 0; offsetIndex < pattern.size(); offsetIndex++) {
						//std::cout << "\n";

						int i = (pattern.rowOffset(offsetIndex) + rowIndex);
						int j = (pattern.columnOffset(offsetIndex) + colIndex);

						// cut-off check
						if (i >= 0 && i < scoreboard->rows && j >= 0 && j < scoreboard->cols) {
							sum += scoreboard->input->at(i*ydim + j);
							factor += pattern.itemWeight(offsetIndex);
						}
					}
					sum = sum / *factor_ptr;
					scoreboard->output->at(rowIndex * ydim + colIndex) = sum;
					sum -= sum;
				}
			}


			// Run In top boundary
			for (; rowIndex < endIndex && rowIndex < xdim; rowIndex++) {
				for (int colIndex = 0; colIndex < ydim; colIndex++) {
					factor = 0;
					for (int offsetIndex = 0; offsetIndex < pattern.size(); offsetIndex++) {
						//std::cout << "\n";

						int i = (pattern.rowOffset(offsetIndex) + rowIndex);
						int j = (pattern.columnOffset(offsetIndex) + colIndex);

						// cut-off check
						if (i >= 0 && i < scoreboard->rows && j >= 0 && j < scoreboard->cols) {
							sum += scoreboard->input->at(i*ydim + j);
							factor += pattern.itemWeight(offsetIndex);
						}
					}
					sum = sum / *factor_ptr;
					scoreboard->output->at(rowIndex * ydim + colIndex) = sum;
					sum -= sum;
				}
			}
		}

		template<typename T>
		void threadVectorNormalOpt3(Scoreboard<std::vector<T>>* scoreboard, size_t startIndex, size_t endIndex) {
			Pattern &pattern = scoreboard->pattern;
			T sum;
			sum = 0;
			int xdim = scoreboard->rows;
			int ydim = scoreboard->cols;
			int colLowBoundary = -scoreboard->pattern.getColumnLowerBoundary();
			int colHighBoundary = scoreboard->cols - scoreboard->pattern.getColumnHigherBoundary();
			int rowLowBoundary = -scoreboard->pattern.getRowLowerBoundary();
			int rowHighBoundary = scoreboard->rows - scoreboard->pattern.getRowHigherBoundary();

			int factor;
			int* factor_ptr = scoreboard->pattern.normalization ? &factor : &no_factor;
			int rowIndex = startIndex;

			// RUN IF IN ROW BOUNDARY
			for (; rowIndex < endIndex && rowIndex < rowLowBoundary; rowIndex++) {
				for (int colIndex = 0; colIndex < ydim; colIndex++) {
					sum = 0;
					factor = 0;
					for (int offsetIndex = 0; offsetIndex < pattern.size(); offsetIndex++) {
						//std::cout << "\n";

						int i = (pattern.rowOffset(offsetIndex) + rowIndex);
						int j = (pattern.columnOffset(offsetIndex) + colIndex);

						// cut-off check
						if (i >= 0 && i < scoreboard->rows && j >= 0 && j < scoreboard->cols) {
							sum += scoreboard->input->at(i*ydim + j);
							factor += pattern.itemWeight(offsetIndex);
						}
					}
					sum = sum / *factor_ptr;
					scoreboard->output->at(rowIndex * ydim + colIndex) = sum;
					sum -= sum;
				}
			}
			sum = 0;
			factor = 0;
			// RUN IF BETWEENBOUNDARIES
			for (; rowIndex < endIndex && rowIndex < rowHighBoundary; rowIndex++) {
				for (int colIndex = 0; colIndex < colLowBoundary; colIndex++) {
					factor = 0;
					sum = 0;
					for (int offsetIndex = 0; offsetIndex < pattern.size(); offsetIndex++) {
						//std::cout << "\n";

						int i = (pattern.rowOffset(offsetIndex) + rowIndex);
						int j = (pattern.columnOffset(offsetIndex) + colIndex);

						// cut-off check
						if (i >= 0 && i < scoreboard->rows && j >= 0 && j < scoreboard->cols) {
							sum += scoreboard->input->at(i*ydim + j);
							factor += pattern.itemWeight(offsetIndex);
						}
					}
					sum = sum / *factor_ptr;
					scoreboard->output->at(rowIndex * ydim + colIndex) = sum;
					sum -= sum;
				}
				sum = 0;
				factor = 0;
				for (int colIndex = colLowBoundary; colIndex < colHighBoundary; colIndex++) {
					sum = 0;
					factor = 0;
					for (int offsetIndex = 0; offsetIndex < pattern.size(); offsetIndex++) {
						int i = (pattern.rowOffset(offsetIndex) + rowIndex);
						int j = (pattern.columnOffset(offsetIndex) + colIndex);
						sum += scoreboard->input->at(i*ydim + j) * pattern.itemWeight(offsetIndex);
						factor += pattern.itemWeight(offsetIndex);
					}
					sum = sum / *factor_ptr;
					scoreboard->output->at(rowIndex * ydim + colIndex) = sum;
				}
				sum = 0;
				factor = 0;
				for (int colIndex = colHighBoundary; colIndex < scoreboard->cols; colIndex++) {
					factor = 0;
					for (int offsetIndex = 0; offsetIndex < pattern.size(); offsetIndex++) {
						//std::cout << "\n";

						int i = (pattern.rowOffset(offsetIndex) + rowIndex);
						int j = (pattern.columnOffset(offsetIndex) + colIndex);

						// cut-off check
						if (i >= 0 && i < scoreboard->rows && j >= 0 && j < scoreboard->cols) {
							sum += scoreboard->input->at(i*ydim + j);
							factor += pattern.itemWeight(offsetIndex);
						}
					}
					sum = sum / *factor_ptr;
					scoreboard->output->at(rowIndex * ydim + colIndex) = sum;
					sum -= sum;
				}
			}
			sum = 0;
			factor = 0;

			// Run In top boundary
			for (; rowIndex < endIndex && rowIndex < xdim; rowIndex++) {
				for (int colIndex = 0; colIndex < ydim; colIndex++) {
					sum = 0;
					factor = 0;
					for (int offsetIndex = 0; offsetIndex < pattern.size(); offsetIndex++) {
						//std::cout << "\n";

						int i = (pattern.rowOffset(offsetIndex) + rowIndex);
						int j = (pattern.columnOffset(offsetIndex) + colIndex);

						// cut-off check
						if (i >= 0 && i < scoreboard->rows && j >= 0 && j < scoreboard->cols) {
							sum += scoreboard->input->at(i*ydim + j);
							factor += pattern.itemWeight(offsetIndex);
						}
					}
					sum = sum / *factor_ptr;
					scoreboard->output->at(rowIndex * ydim + colIndex) = sum;
					sum -= sum;
				}
			}
		}
		
		template<typename T>
		void threadVectorNormalOpt4(Scoreboard<std::vector<T>>* scoreboard, size_t startIndex, size_t endIndex) {
			Pattern &pattern = scoreboard->pattern;
			T sum;
			sum = 0;
			int xdim = scoreboard->rows;
			int ydim = scoreboard->cols;
			int colLowBoundary = -scoreboard->pattern.getColumnLowerBoundary();
			int colHighBoundary = scoreboard->cols - scoreboard->pattern.getColumnHigherBoundary();
			int rowLowBoundary = -scoreboard->pattern.getRowLowerBoundary();
			int rowHighBoundary = scoreboard->rows - scoreboard->pattern.getRowHigherBoundary();

			int factor;
			int* factor_ptr = scoreboard->pattern.normalization ? &factor : &no_factor;
			int rowIndex = startIndex;

			// RUN IF IN ROW BOUNDARY
			for (; rowIndex < endIndex && rowIndex < rowLowBoundary; rowIndex++) {
				for (int colIndex = 0; colIndex < ydim; colIndex++) {
					sum = 0;
					factor = 0;
					for (int offsetIndex = 0; offsetIndex < pattern.size(); offsetIndex++) {
						//std::cout << "\n";

						int i = (pattern.rowOffset(offsetIndex) + rowIndex);
						int j = (pattern.columnOffset(offsetIndex) + colIndex);

						// cut-off check
						if (i >= 0 && i < scoreboard->rows && j >= 0 && j < scoreboard->cols) {
							sum += scoreboard->input->at(i*ydim + j);
							factor += pattern.itemWeight(offsetIndex);
						}
					}
					sum = sum / *factor_ptr;
					scoreboard->output->at(rowIndex * ydim + colIndex) = sum;
					sum -= sum;
				}
			}

			// RUN IF BETWEENBOUNDARIES
			for (; rowIndex < endIndex && rowIndex < rowHighBoundary; rowIndex++) {
				for (int colIndex = 0; colIndex < colLowBoundary; colIndex++) {
					factor = 0;
					sum = 0;
					for (int offsetIndex = 0; offsetIndex < pattern.size(); offsetIndex++) {
						//std::cout << "\n";

						int i = (pattern.rowOffset(offsetIndex) + rowIndex);
						int j = (pattern.columnOffset(offsetIndex) + colIndex);

						// cut-off check
						if (i >= 0 && i < scoreboard->rows && j >= 0 && j < scoreboard->cols) {
							sum += scoreboard->input->at(i*ydim + j);
							factor += pattern.itemWeight(offsetIndex);
						}
					}
					sum = sum / *factor_ptr;
					scoreboard->output->at(rowIndex * ydim + colIndex) = sum;
					sum -= sum;
				}

				for (int colIndex = colHighBoundary; colIndex < scoreboard->cols; colIndex++) {
					factor = 0;
					for (int offsetIndex = 0; offsetIndex < pattern.size(); offsetIndex++) {
						//std::cout << "\n";

						int i = (pattern.rowOffset(offsetIndex) + rowIndex);
						int j = (pattern.columnOffset(offsetIndex) + colIndex);

						// cut-off check
						if (i >= 0 && i < scoreboard->rows && j >= 0 && j < scoreboard->cols) {
							sum += scoreboard->input->at(i*ydim + j);
							factor += pattern.itemWeight(offsetIndex);
						}
					}
					sum = sum / *factor_ptr;
					scoreboard->output->at(rowIndex * ydim + colIndex) = sum;
					sum -= sum;
				}
			}


			// Run In top boundary
			for (; rowIndex < endIndex && rowIndex < xdim; rowIndex++) {
				for (int colIndex = 0; colIndex < ydim; colIndex++) {
					sum = 0;
					factor = 0;
					for (int offsetIndex = 0; offsetIndex < pattern.size(); offsetIndex++) {
						//std::cout << "\n";

						int i = (pattern.rowOffset(offsetIndex) + rowIndex);
						int j = (pattern.columnOffset(offsetIndex) + colIndex);

						// cut-off check
						if (i >= 0 && i < scoreboard->rows && j >= 0 && j < scoreboard->cols) {
							sum += scoreboard->input->at(i*ydim + j);
							factor += pattern.itemWeight(offsetIndex);
						}
					}
					sum = sum / *factor_ptr;
					scoreboard->output->at(rowIndex * ydim + colIndex) = sum;
					sum -= sum;
				}
			}
		}

		// Functionality: Border - normal functionality but does not touch boundaries
		// --------------------------------------------------------------------------
		template<typename T>
		void threadBorder(Scoreboard<T>* scoreboard, size_t startIndex, size_t endIndex) {
			//std::cout << __PRETTY_FUNCTION__ << std::endl;
			Pattern &pattern = scoreboard->pattern;
			T sum;
			sum -= sum;
			int factor;
			int* factor_ptr = scoreboard->pattern.normalization ? &factor : &no_factor;
			for (int rowIndex = startIndex; rowIndex < endIndex; rowIndex++) {
				for (int colIndex = -pattern.getColumnLowerBoundary(); colIndex < scoreboard->cols- pattern.getColumnHigherBoundary(); colIndex++) {
					factor = 0;
					for (int offsetIndex = 0; offsetIndex < pattern.size(); offsetIndex++) {
						int i = (pattern.rowOffset(offsetIndex) + rowIndex);
						int j = (pattern.columnOffset(offsetIndex) + colIndex);
						sum += (*(scoreboard->input + i * scoreboard->cols + j)) * pattern.itemWeight(offsetIndex);
						factor += pattern.itemWeight(offsetIndex);
					}

					sum = sum / *factor_ptr;
					*(scoreboard->output + rowIndex * scoreboard->cols + colIndex) = sum;
					sum -= sum;
				}
			}
		}

		template<typename T>
		void threadVectorBorder(Scoreboard<std::vector<T>>* scoreboard, size_t startIndex, size_t endIndex) {
			// std::cout << __PRETTY_FUNCTION__ << std::endl;
			Pattern &pattern = scoreboard->pattern;
			T sum;
			int xdim = scoreboard->rows;
			int ydim = scoreboard->cols;

			int factor;
			int* factor_ptr = scoreboard->pattern.normalization ? &factor : &no_factor;
			for (int rowIndex = startIndex; rowIndex < endIndex; rowIndex++) {
				for (int colIndex = -pattern.getColumnLowerBoundary(); colIndex < scoreboard->cols - pattern.getColumnHigherBoundary(); colIndex++) {
					sum = 0;
			//		std::cout << "\n\n\n =============================================================================================\n";
			//		 std::cout << "Pixel Main(" << rowIndex << ", " << colIndex << ") color: " << scoreboard->input->at(rowIndex * ydim + colIndex) << std::endl;
			//		std::cout << " Zero (" << rowIndex << ", " << colIndex << ") color: " << sum <<  " and Factor = " << factor << std::endl;
//
					factor = 0;
					for (int offsetIndex = 0; offsetIndex < pattern.size(); offsetIndex++) {
							// std::cout << "\n";
						int i = (pattern.rowOffset(offsetIndex) + rowIndex);
						int j = (pattern.columnOffset(offsetIndex) + colIndex);
				//		std::cout << "OS (" << pattern.rowOffset(offsetIndex) << ", " << pattern.columnOffset(offsetIndex) << ", " << pattern.itemWeight(offsetIndex) << ")  ";
			//			std::cout << "Pixel (" << i << ", " << j << ") color: " << scoreboard->input->at(i * ydim + j) << std::endl;
						// std::cout << "WITH weight = " << scoreboard->pattern.itemWeight(offsetIndex)<<std::endl;
						sum += scoreboard->input->at(i*ydim + j) * scoreboard->pattern.itemWeight(offsetIndex);
						factor += pattern.itemWeight(offsetIndex);

				//		std::cout << "Sum Pixel color: " << sum << " and Factor = " << factor << std::endl;

					}
					sum = sum / *factor_ptr;
			//		  std::cout << "Sum Out(" << rowIndex << ", " << colIndex << ") color: " << sum << " and Factor = " << factor << std::endl;
//
					scoreboard->output->at(rowIndex * ydim + colIndex) = sum;

				}
			}
		}


		// Constructor
		// -----------
		StencilImplementation(size_t threads = 0)  {
			nthreads = threads ? threads : std::thread::hardware_concurrency();
			std::cout << "Stencil: Initialised with " << threads << " threads\n";
		}

		// Utils
		void generateThreadData(Pattern so, BoundaryHandling bh, int rows) {
			THREADS = std::vector <std::thread *>(nthreads);
			if (bh == BoundaryHandling::BORDER || bh == BoundaryHandling::NORMAL_OPT_1 || bh == BoundaryHandling::NORMAL_OPT_2 || bh == BoundaryHandling::NORMAL_OPT_4) {
				num_heavier_threads = (rows - so.getRowHigherBoundary() + so.getRowLowerBoundary()) % nthreads;
				rows_per_thread = ((rows - so.getRowHigherBoundary() + so.getRowLowerBoundary()) / nthreads ) + 1;
				index_begin = -so.getRowLowerBoundary();
				index_end = 0;
			}
			else {
				num_heavier_threads = ( rows % nthreads );
				rows_per_thread = rows / nthreads + 1;
				index_begin = 0;
				index_end = 0;
			}

		//	std::cout <<nthreads << " "<< index_begin << " " << " " << num_heavier_threads << " " << rows_per_thread << std::endl;
		}
	public:
		// Paranthesis operator: call function
		// -----------------------------------
		template<size_t rows, size_t cols, typename T>
		void operator()(T(&output)[rows][cols], T(&input)[rows][cols], Pattern &pattern, BoundaryHandling bh = BoundaryHandling::NORMAL) {

			scoreboard = new Scoreboard<T>(pattern, (T*)input, (T*)output, rows, cols);
			generateThreadData(pattern,bh, rows);

			// Run threads
			// -----------
			// Duplicative code, however, only 1 check for Boundary Handling
			if (bh == BoundaryHandling::WRAP) {

				for (t = 0; t < num_heavier_threads; t++) {
					index_end = index_begin + rows_per_thread;
			//		std::cout << "F1 -> Thread: (" << t << ", " << index_begin << ", " << index_end << ")\n";
					THREADS.at(t) = new std::thread(&StencilImplementation::threadWrap<T>, this, (Scoreboard<T>*)scoreboard, index_begin, index_end);
					index_begin += rows_per_thread;
				}
				rows_per_thread--;
				for (;t<nthreads; t++){
					index_end = index_begin + rows_per_thread;
			//		std::cout << "F2 -> Thread: (" << t << ", " << index_begin << ", " << index_end << ")\n";
					THREADS.at(t) = new std::thread(&StencilImplementation::threadWrap<T>, this, (Scoreboard<T>*)scoreboard, index_begin, index_end);
					index_begin += rows_per_thread;
				}
			}
			else if (bh == BoundaryHandling::MIRROR) {
				// CAN OPTIMIZE IN TWO RUNS for boundary ones and non
				for (t = 0; t < num_heavier_threads; t++) {
					index_end = index_begin + rows_per_thread;
			//		std::cout << "F1 -> Thread: (" << t << ", " << index_begin << ", " << index_end << ")\n";
					THREADS.at(t) = new std::thread(&StencilImplementation::threadMirror<T>, this, (Scoreboard<T>*)scoreboard, index_begin, index_end);
					index_begin += rows_per_thread ;
				}
				rows_per_thread--;
				for (;t<nthreads; t++){
					index_end = index_begin + rows_per_thread;
			//		std::cout << "F2 -> Thread: (" << t << ", " << index_begin << ", " << index_end << ")\n";
					THREADS.at(t) = new std::thread(&StencilImplementation::threadMirror<T>, this, (Scoreboard<T>*)scoreboard, index_begin, index_end);
					index_begin += rows_per_thread;
				}
			}
			else if (bh == BoundaryHandling::BORDER) {

				for (t = 0; t < num_heavier_threads; t++) {
					index_end = index_begin + rows_per_thread;
			//		std::cout << "F1 -> Thread: (" << t << ", " << index_begin << ", " << index_end << ")\n";
					THREADS.at(t) = new std::thread(&StencilImplementation::threadBorder<T>, this, (Scoreboard<T>*)scoreboard, index_begin, index_end);
					index_begin += rows_per_thread;
				}
				rows_per_thread--;
				for (;t<nthreads; t++){
					index_end = index_begin + rows_per_thread;
			//		std::cout << "F2 -> Thread: (" << t << ", " << index_begin << ", " << index_end << ")\n";
					THREADS.at(t) = new std::thread(&StencilImplementation::threadBorder<T>, this, (Scoreboard<T>*)scoreboard, index_begin, index_end);
					index_begin += rows_per_thread;
				}
			}
			else { // NORMAL BOUNDARY HANDLING
				// CAN OPTIMIZE IN TWO RUNS for boundary ones and non
				for (t = 0; t < num_heavier_threads; t++) {
					index_end = index_begin + rows_per_thread;
			//		std::cout << "F1 -> Thread: (" << t << ", " << index_begin << ", " << index_end << ")\n";
					THREADS.at(t) = new std::thread(&StencilImplementation::threadNormal<T>, this, (Scoreboard<T>*)scoreboard, index_begin, index_end);
					index_begin += rows_per_thread;
				}
				rows_per_thread--;
				for (;t<nthreads; t++){
					index_end = index_begin + rows_per_thread;
			//		std::cout << "F2 -> Thread: (" << t << ", " << index_begin << ", " << index_end << ")\n";
					THREADS.at(t) = new std::thread(&StencilImplementation::threadNormal<T>, this, (Scoreboard<T>*)scoreboard, index_begin, index_end);
					index_begin += rows_per_thread;
				}
			}

			// Join threads
			// ------------
			for (size_t t = 0; t < nthreads; ++t) { THREADS[t]->join(); delete THREADS[t]; }

		}

		template< typename T>
		void operator()(std::vector<T> &output, std::vector<T> &input, Pattern &pattern, BoundaryHandling bh = BoundaryHandling::NORMAL, int xdim = 1, int ydim = 1) {
		//	std::cout << "HAAR: " << xdim << " " << ydim<< std::endl;
			scoreboard = new Scoreboard<std::vector<T>>(pattern, &input, &output, xdim, ydim);
			generateThreadData(pattern, bh, xdim);


		//	std::cout << "Img sizes: \n" ;
			if (bh == BoundaryHandling::WRAP){
				for (t = 0; t < num_heavier_threads; t++) {
					index_end = index_begin + rows_per_thread;
					THREADS.at(t) = new std::thread(&StencilImplementation::threadVectorWrap<T>, this, (Scoreboard<std::vector<T>>*)scoreboard, index_begin, index_end);

					index_begin += rows_per_thread;
				}
				rows_per_thread--;
				for (; t < nthreads; t++) {
					index_end = index_begin + rows_per_thread;
					THREADS.at(t) = new std::thread(&StencilImplementation::threadVectorWrap<T>, this, (Scoreboard<std::vector<T>>*)scoreboard, index_begin, index_end);

					index_begin += rows_per_thread;
				}
			}
			else if (bh == BoundaryHandling::MIRROR) {
				for (t = 0; t < num_heavier_threads; t++) {
					index_end = index_begin + rows_per_thread;
					THREADS.at(t) = new std::thread(&StencilImplementation::threadVectorMirror<T>, this, (Scoreboard<std::vector<T>>*)scoreboard, index_begin, index_end);

					index_begin += rows_per_thread;
				}
				rows_per_thread--;
				for (; t < nthreads; t++) {
					index_end = index_begin + rows_per_thread;
					THREADS.at(t) = new std::thread(&StencilImplementation::threadVectorMirror<T>, this, (Scoreboard<std::vector<T>>*)scoreboard, index_begin, index_end);

					index_begin += rows_per_thread;
				}
			}
			else if (bh == BoundaryHandling::BORDER) {
				for (t = 0; t < num_heavier_threads; t++) {
					index_end = index_begin + rows_per_thread;
					THREADS.at(t) = new std::thread(&StencilImplementation::threadVectorBorder<T>, this, (Scoreboard<std::vector<T>>*)scoreboard, index_begin, index_end);

					index_begin += rows_per_thread;
				}
				rows_per_thread--;
				for (; t < nthreads; t++) {
					index_end = index_begin + rows_per_thread;
					THREADS.at(t) = new std::thread(&StencilImplementation::threadVectorBorder<T>, this, (Scoreboard<std::vector<T>>*)scoreboard, index_begin, index_end);

					index_begin += rows_per_thread;
				}
			}
			else if (bh == BoundaryHandling::NORMAL_OPT_1) {
				// run as BORDER
				for (t = 0; t < num_heavier_threads; t++) {
					index_end = index_begin + rows_per_thread;
					THREADS.at(t) = new std::thread(&StencilImplementation::threadVectorBorder<T>, this, (Scoreboard<std::vector<T>>*)scoreboard, index_begin, index_end);
				//	std::cout << "FROM F1: " << index_begin << " " << index_end << std::endl;
					index_begin += rows_per_thread;
				}
				rows_per_thread--;
				for (; t < nthreads; t++) {
					index_end = index_begin + rows_per_thread;
					THREADS.at(t) = new std::thread(&StencilImplementation::threadVectorBorder<T>, this, (Scoreboard<std::vector<T>>*)scoreboard, index_begin, index_end);

					index_begin += rows_per_thread;
				}

					//	std::cout << "HERE" << std::endl;

				// run as ONLYBORDER
				for (size_t t = 0; t < nthreads; ++t) { THREADS[t]->join(); delete THREADS[t]; }
				bh = BoundaryHandling::NORMAL;
				generateThreadData(pattern, bh, xdim);
				for (t = 0; t < num_heavier_threads; t++) {
					index_end = index_begin + rows_per_thread;
					THREADS.at(t) = new std::thread(&StencilImplementation::threadVectorNormalOpt1<T>, this, (Scoreboard<std::vector<T>>*)scoreboard, index_begin, index_end);

					index_begin += rows_per_thread;
				}
				rows_per_thread--;
				for (; t < nthreads; t++) {
					index_end = index_begin + rows_per_thread;
					THREADS.at(t) = new std::thread(&StencilImplementation::threadVectorNormalOpt1<T>, this, (Scoreboard<std::vector<T>>*)scoreboard, index_begin, index_end);

					index_begin += rows_per_thread;
				}

			}
			else if (bh == BoundaryHandling::NORMAL_OPT_2) {
				// TRY 2
			size_t num_heavier_threadsBL = num_heavier_threads;
			size_t rows_per_threadBL = rows_per_thread;
			size_t index_beginBL = index_begin;
			size_t index_endBL = index_end;
			bh = BoundaryHandling::NORMAL;
			generateThreadData(pattern, bh, xdim);
			size_t t;
			for (t = 0; t < nthreads; t++) {
				if (num_heavier_threads == 0) { rows_per_thread--; }
				if (num_heavier_threadsBL == 0) { rows_per_threadBL--; }
				index_end = index_begin + rows_per_thread;
				index_endBL = index_beginBL + rows_per_threadBL;

			//	std::cout << index_beginBL <<" " << index_endBL-1 << " " << index_begin << " " << index_end -1  << "\n";
				THREADS.at(t) = new std::thread(&StencilImplementation::threadVectorNormalOpt2<T>, this, (Scoreboard<std::vector<T>>*)scoreboard, index_begin, index_end, index_beginBL, index_endBL);

				index_begin += rows_per_thread;
				index_beginBL += rows_per_threadBL;

				num_heavier_threads--;
				num_heavier_threadsBL--;
			}
			}
			else if (bh == BoundaryHandling::NORMAL_OPT_3) {
				for (t = 0; t < num_heavier_threads; t++) {
					index_end = index_begin + rows_per_thread;
					THREADS.at(t) = new std::thread(&StencilImplementation::threadVectorNormalOpt3<T>, this, (Scoreboard<std::vector<T>>*)scoreboard, index_begin, index_end);

					index_begin += rows_per_thread;
				}
				rows_per_thread--;
				for (; t < nthreads; t++) {
					index_end = index_begin + rows_per_thread;
					THREADS.at(t) = new std::thread(&StencilImplementation::threadVectorNormalOpt3<T>, this, (Scoreboard<std::vector<T>>*)scoreboard, index_begin, index_end);

					index_begin += rows_per_thread;
				}
			}
			else if (bh == BoundaryHandling::NORMAL_OPT_4) {
				// run as BORDER
				for (t = 0; t < num_heavier_threads; t++) {
					index_end = index_begin + rows_per_thread;
					THREADS.at(t) = new std::thread(&StencilImplementation::threadVectorBorder<T>, this, (Scoreboard<std::vector<T>>*)scoreboard, index_begin, index_end);
					//	std::cout << "FROM F1: " << index_begin << " " << index_end << std::endl;
					index_begin += rows_per_thread;
				}
				rows_per_thread--;
				for (; t < nthreads; t++) {
					index_end = index_begin + rows_per_thread;
					THREADS.at(t) = new std::thread(&StencilImplementation::threadVectorBorder<T>, this, (Scoreboard<std::vector<T>>*)scoreboard, index_begin, index_end);

					index_begin += rows_per_thread;
				}

				//	std::cout << "HERE" << std::endl;


				// run as ONLYBORDER
			//	for (size_t t = 0; t < nthreads; ++t) { THREADS[t]->join(); delete THREADS[t]; }
				int items_in_cows = ydim + pattern.getColumnLowerBoundary() - pattern.getRowHigherBoundary();
				int items_in_rows = xdim + pattern.getRowLowerBoundary() - pattern.getRowHigherBoundary();
				int borderitems = xdim * ydim - items_in_cows * items_in_rows;
				int meanItemsPerThread = borderitems / nthreads + 1;

			//	std::cout << items_in_cows <<" "<< items_in_rows << " " << borderitems << " " << meanItemsPerThread << "\n";
				index_begin = 0;
				index_end = 0;
				for (t = 0; t < nthreads - 1; t++) {
					int nextitems = 0;
					while (nextitems < meanItemsPerThread) {
						if (index_end < -pattern.getRowLowerBoundary()) nextitems += ydim;
						else if (index_end < pattern.getColumnHigherBoundary()) nextitems += ydim - items_in_cows;
						else nextitems += ydim;
						index_end++;
					}
					THREADS.at(t) = new std::thread(&StencilImplementation::threadVectorNormalOpt4<T>, this, (Scoreboard<std::vector<T>>*)scoreboard, index_begin, index_end);
					index_begin = index_end;
				}
				THREADS.at(nthreads - 1) = new std::thread(&StencilImplementation::threadVectorNormalOpt4<T>, this, (Scoreboard<std::vector<T>>*)scoreboard, index_begin, xdim);

			}
			else {
				for (t = 0; t < num_heavier_threads; t++) {
					index_end = index_begin + rows_per_thread;
					THREADS.at(t) = new std::thread(&StencilImplementation::threadVectorNormal<T>, this, (Scoreboard<std::vector<T>>*)scoreboard, index_begin, index_end);

					index_begin += rows_per_thread;
				}
				rows_per_thread--;
				for (; t < nthreads; t++) {
					index_end = index_begin + rows_per_thread;
					THREADS.at(t) = new std::thread(&StencilImplementation::threadVectorNormal<T>, this, (Scoreboard<std::vector<T>>*)scoreboard, index_begin, index_end);

					index_begin += rows_per_thread;
				}
			}
			for (size_t t = 0; t < nthreads; ++t) { THREADS[t]->join(); delete THREADS[t]; }
		}

		// Friend Functions for Stencil Implementation Class
		// ---------------------------------------------
		friend StencilImplementation __StencilWithAccess(const size_t &threads);
	};

};

/*
* We cannot define a friend function with default argument
* that needs access to inner class on latest g++ compiler versions.
* We need a wrapper!
*/
StencilSkeleton::StencilImplementation __StencilWithAccess(const size_t &threads) {
	return StencilSkeleton::StencilImplementation(threads);
}

StencilSkeleton::StencilImplementation Stencil(const size_t &threads = 0) {
	return __StencilWithAccess(threads);
}

//}
#endif // STENCIL_HPP
