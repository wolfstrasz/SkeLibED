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
#define PSLED_CROP StencilSkeleton::BoundaryHandling::CROP
#define PSLED_NORMAL_OPT2 StencilSkeleton::BoundaryHandling::NORMAL_OPT_2
#define PSLED_NORMAL_OPT3 StencilSkeleton::BoundaryHandling::NORMAL_OPT_3

class StencilSkeleton {

private:
	StencilSkeleton() {}

public:
	// Enumerator class used to choose what kind of boundary handling the Stencil should use.
	// --------------------------------------------------------------------------------------
	enum class BoundaryHandling {
		NORMAL,
		WRAP,
		MIRROR,
		CROP,
		NORMAL_OPT_2,
		NORMAL_OPT_3
	};

	// StencilImplementation
	// ---------------------
	class StencilImplementation {
	private:

		template <typename T>
		class Scoreboard {
		public:
			Pattern pattern;
			int rows;
			int cols;
			T* input;
			T* output;
			Scoreboard(Pattern &pattern, T * input, T * output, int rows = 1, int cols = 1) {
				this->pattern = pattern;
				this->input = input;
				this->output = output;
				this->rows = rows;
				this->cols = cols;
			}
		};

		std::vector<std::thread *> THREADS;
		int no_factor = 1;								// used when there should be no normalization (read-only)
		size_t nthreads;								// number of threads used at run
		size_t num_heavier_threads;						// number of threads that own more rows (i.e. more work to do)
		size_t rows_per_thread;
		size_t index_begin;
		size_t index_end;
		size_t t;

		// Functionality: Wrap - Uses wrapping around boundaries
		// -----------------------------------------------------
		template<typename T>
		void threadVectorWrap(Scoreboard<std::vector<T>>* scoreboard, size_t startIndex, size_t endIndex) {
			//std::cout << __PRETTY_FUNCTION__ << std::endl;
			// Initialise local data
			Pattern &pattern = scoreboard->pattern;
			T sum;
			int factor;
			int xdim = scoreboard->rows;
			int ydim = scoreboard->cols;

			// Choose if no normalization wil be done (i.e. averaging by weight)
			int* factor_ptr = scoreboard->pattern.normalization ? &factor : &no_factor;

			// Execution
			for (int rowIndex = startIndex; rowIndex < endIndex; rowIndex++) {
				for (int colIndex = 0; colIndex < scoreboard->cols; colIndex++) {
					factor = 0;
					for (int offsetIndex = 0; offsetIndex < pattern.size(); offsetIndex++) {
						int i = (pattern.rowOffset(offsetIndex) + rowIndex);
						int j = (pattern.columnOffset(offsetIndex) + colIndex);
						// wrapping
						i = (i + xdim) % xdim; // addition required because -4 % 5 is not = 1 by definition of % operator
						j = (j + ydim) % ydim;
						sum += scoreboard->input->at(i*ydim + j)* pattern.itemWeight(offsetIndex);
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
		void threadVectorMirror(Scoreboard<std::vector<T>>* scoreboard, size_t startIndex, size_t endIndex) {
			//std::cout << __PRETTY_FUNCTION__ << std::endl;

			// Initialise local data
			Pattern &pattern = scoreboard->pattern;
			T sum;
			int factor;
			int xdim = scoreboard->rows;
			int ydim = scoreboard->cols;

			// Choose if no normalization wil be done (i.e. averaging by weight)
			int* factor_ptr = scoreboard->pattern.normalization ? &factor : &no_factor;

			// Execution
			for (int rowIndex = startIndex; rowIndex < endIndex; rowIndex++) {
				for (int colIndex = 0; colIndex < scoreboard->cols; colIndex++) {
					sum = 0;
					factor = 0;
					for (int offsetIndex = 0; offsetIndex < pattern.size(); offsetIndex++) {
						int i = (pattern.rowOffset(offsetIndex) + rowIndex);
						int j = (pattern.columnOffset(offsetIndex) + colIndex);
						// mirroring
						if (i < 0 || i >= xdim) i = (rowIndex - pattern.rowOffset(offsetIndex));
						if (j < 0 || j >= ydim) j = (colIndex - pattern.columnOffset(offsetIndex));

						sum += scoreboard->input->at(i*ydim + j)* pattern.itemWeight(offsetIndex);
						factor += pattern.itemWeight(offsetIndex);
					}
					sum = sum / *factor_ptr;
					scoreboard->output->at(rowIndex * ydim + colIndex) = sum;
				}
			}
		}

		// Functionality: Normal - Does not use other values to exchange unobtainable offset items (cut-off)
		// -------------------------------------------------------------------------------------------------
		template<typename T>
		void threadVectorNormal(Scoreboard<std::vector<T>>* scoreboard, size_t startIndex, size_t endIndex) {
			//std::cout << __PRETTY_FUNCTION__ << std::endl;

			// Initialise local data
			Pattern &pattern = scoreboard->pattern;
			T sum;
			int factor;
			int xdim = scoreboard->rows;
			int ydim = scoreboard->cols;

			// Choose if no normalization wil be done (i.e. averaging by weight)
			int* factor_ptr = scoreboard->pattern.normalization ? &factor : &no_factor;

			// Execution
			// ------------------------------------------------------------------------------------
			for (int rowIndex = startIndex; rowIndex < endIndex; rowIndex++) {
				for (int colIndex = 0; colIndex < scoreboard->cols; colIndex++) {
					sum = 0;
					factor = 0;
					for (int offsetIndex = 0; offsetIndex < pattern.size(); offsetIndex++) {

						int i = (pattern.rowOffset(offsetIndex) + rowIndex);
						int j = (pattern.columnOffset(offsetIndex) + colIndex);

						// cut-off check
						if (i >= 0 && i < xdim && j >= 0 && j < ydim) {
							sum += scoreboard->input->at(i*ydim + j) * pattern.itemWeight(offsetIndex);
							factor += pattern.itemWeight(offsetIndex);
						}
					}
					sum = sum / *factor_ptr;
					scoreboard->output->at(rowIndex * ydim + colIndex) = sum;
				}
			}
		}

		template<typename T>
		void threadVectorNormalOpt2(Scoreboard<std::vector<T>>* scoreboard, size_t startIndex, size_t endIndex, size_t startIndexBL, size_t endIndexBL) {
			//std::cout << __PRETTY_FUNCTION__ << std::endl;

			// Initialise local data
			Pattern &pattern = scoreboard->pattern;
			T sum;
			int factor;
			int xdim = scoreboard->rows;
			int ydim = scoreboard->cols;
			int colLowBoundary = -scoreboard->pattern.getColumnLowerBoundary();
			int colHighBoundary = scoreboard->cols - scoreboard->pattern.getColumnHigherBoundary();
			int rowLowBoundary = -scoreboard->pattern.getRowLowerBoundary();
			int rowHighBoundary = scoreboard->rows - scoreboard->pattern.getRowHigherBoundary();

			// Decide margins for different column functionality with respect to rows dimensionality 
			int lower = endIndex < rowLowBoundary ? endIndex : rowLowBoundary;
			int middle = endIndex < rowHighBoundary ? endIndex : rowHighBoundary;
			int upper = endIndex < xdim ? endIndex : xdim;

			// Choose if no normalization wil be done (i.e. averaging by weight)
			int* factor_ptr = scoreboard->pattern.normalization ? &factor : &no_factor;

			// Execution
			// ------------------------------------------------------------------------------------
			// RUN ON CROP MODE
			// --------------------------------------
			int rowIndex = startIndexBL;
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
			// RUN ON NORMAL MODE
			// --------------------------------------
			rowIndex = startIndex;
			// Bottom rows: all columns are affected by boundary handling
			for (; rowIndex < lower; rowIndex++) {
				for (int colIndex = 0; colIndex < ydim; colIndex++) {
					sum = 0;
					factor = 0;
					for (int offsetIndex = 0; offsetIndex < pattern.size(); offsetIndex++) {
						int i = (pattern.rowOffset(offsetIndex) + rowIndex);
						int j = (pattern.columnOffset(offsetIndex) + colIndex);

						// cut-off check
						if (i >= 0 && i < xdim && j >= 0 && j < ydim) {
							sum += scoreboard->input->at(i*ydim + j) * pattern.itemWeight(offsetIndex);
							factor += pattern.itemWeight(offsetIndex);
						}
					}
					sum = sum / *factor_ptr;
					scoreboard->output->at(rowIndex * ydim + colIndex) = sum;
				}
			}

			// Middle rows: only columns affected by boundary handling
			for (; rowIndex < middle; rowIndex++) {
				for (int colIndex = 0; colIndex < colLowBoundary; colIndex++) {
					sum = 0;
					factor = 0;
					for (int offsetIndex = 0; offsetIndex < pattern.size(); offsetIndex++) {
						int i = (pattern.rowOffset(offsetIndex) + rowIndex);
						int j = (pattern.columnOffset(offsetIndex) + colIndex);

						// cut-off check
						if (i >= 0 && i < xdim && j >= 0 && j < ydim) {
							sum += scoreboard->input->at(i*ydim + j)* pattern.itemWeight(offsetIndex);
							factor += pattern.itemWeight(offsetIndex);
						}
					}
					sum = sum / *factor_ptr;
					scoreboard->output->at(rowIndex * ydim + colIndex) = sum;
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
				}
			}

			// Run In top boundary
			for (; rowIndex < upper; rowIndex++) {
				for (int colIndex = 0; colIndex < ydim; colIndex++) {
					sum = 0;
					factor = 0;
					for (int offsetIndex = 0; offsetIndex < pattern.size(); offsetIndex++) {
						int i = (pattern.rowOffset(offsetIndex) + rowIndex);
						int j = (pattern.columnOffset(offsetIndex) + colIndex);

						// cut-off check
						if (i >= 0 && i < scoreboard->rows && j >= 0 && j < scoreboard->cols) {
							sum += scoreboard->input->at(i*ydim + j)* pattern.itemWeight(offsetIndex);
							factor += pattern.itemWeight(offsetIndex);
						}
					}
					sum = sum / *factor_ptr;
					scoreboard->output->at(rowIndex * ydim + colIndex) = sum;
				}
			}
		}

		template<typename T>
		void threadVectorNormalOpt3(Scoreboard<std::vector<T>>* scoreboard, size_t startIndex, size_t endIndex) {
			//std::cout << __PRETTY_FUNCTION__ << std::endl;

			// Initialise local data
			Pattern &pattern = scoreboard->pattern;
			T sum;
			int factor;
			int xdim = scoreboard->rows;
			int ydim = scoreboard->cols;
			int colLowBoundary = -scoreboard->pattern.getColumnLowerBoundary();
			int colHighBoundary = scoreboard->cols - scoreboard->pattern.getColumnHigherBoundary();
			int rowLowBoundary = -scoreboard->pattern.getRowLowerBoundary();
			int rowHighBoundary = scoreboard->rows - scoreboard->pattern.getRowHigherBoundary();
			int rowIndex = startIndex;

			// Decide margins for different column functionality with respect to rows dimensionality 
			int lower = endIndex < rowLowBoundary ? endIndex : rowLowBoundary;
			int middle = endIndex < rowHighBoundary ? endIndex : rowHighBoundary;
			int upper = endIndex < xdim ? endIndex : xdim;

			// Choose if no normalization wil be done (i.e. averaging by weight)
			int* factor_ptr = scoreboard->pattern.normalization ? &factor : &no_factor;

			// Execution
			// ------------------------------------------------------------------------------------

			// Bottom rows: all columns are affected by boundary handling
			for (; rowIndex < lower; rowIndex++) {
				for (int colIndex = 0; colIndex < ydim; colIndex++) {
					sum = 0;
					factor = 0;
					for (int offsetIndex = 0; offsetIndex < pattern.size(); offsetIndex++) {
						//std::cout << "\n";

						int i = (pattern.rowOffset(offsetIndex) + rowIndex);
						int j = (pattern.columnOffset(offsetIndex) + colIndex);

						// cut-off check
						if (i >= 0 && i < scoreboard->rows && j >= 0 && j < scoreboard->cols) {
							sum += scoreboard->input->at(i*ydim + j) * pattern.itemWeight(offsetIndex);
							factor += pattern.itemWeight(offsetIndex);
						}
					}
					sum = sum / *factor_ptr;
					scoreboard->output->at(rowIndex * ydim + colIndex) = sum;
				}
			}
			
			// Middle rows: some columns are affected by boundary handling
			for (; rowIndex < middle; rowIndex++) {
				// Left border columns
				for (int colIndex = 0; colIndex < colLowBoundary; colIndex++) {
					factor = 0;
					sum = 0;
					for (int offsetIndex = 0; offsetIndex < pattern.size(); offsetIndex++) {
						//std::cout << "\n";

						int i = (pattern.rowOffset(offsetIndex) + rowIndex);
						int j = (pattern.columnOffset(offsetIndex) + colIndex);

						// cut-off check
						if (i >= 0 && i < xdim && j >= 0 && j < ydim ) {
							sum += scoreboard->input->at(i*ydim + j)* pattern.itemWeight(offsetIndex);
							factor += pattern.itemWeight(offsetIndex);
						}
					}
					sum = sum / *factor_ptr;
					scoreboard->output->at(rowIndex * ydim + colIndex) = sum;
				}
				
				// Middle columns not affected by boundary handling
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

				// Right border columns
				for (int colIndex = colHighBoundary; colIndex < scoreboard->cols; colIndex++) {
					sum = 0;
					factor = 0;
					for (int offsetIndex = 0; offsetIndex < pattern.size(); offsetIndex++) {
						//std::cout << "\n";

						int i = (pattern.rowOffset(offsetIndex) + rowIndex);
						int j = (pattern.columnOffset(offsetIndex) + colIndex);

						// cut-off check
						if (i >= 0 && i < xdim && j >= 0 && j < ydim) {
							sum += scoreboard->input->at(i*ydim + j)* pattern.itemWeight(offsetIndex);
							factor += pattern.itemWeight(offsetIndex);
						}
					}
					sum = sum / *factor_ptr;
					scoreboard->output->at(rowIndex * ydim + colIndex) = sum;
				}
			}
		
			// Top rows: all columns are affected by boundary handling
			for (; rowIndex < upper; rowIndex++) {
				for (int colIndex = 0; colIndex < ydim; colIndex++) {
					sum = 0;
					factor = 0;
					for (int offsetIndex = 0; offsetIndex < pattern.size(); offsetIndex++) {
						int i = (pattern.rowOffset(offsetIndex) + rowIndex);
						int j = (pattern.columnOffset(offsetIndex) + colIndex);

						// cut-off check
						if (i >= 0 && i < xdim && j >= 0 && j < ydim) {
							sum += scoreboard->input->at(i*ydim + j);
							factor += pattern.itemWeight(offsetIndex)* pattern.itemWeight(offsetIndex);
						}
					}
					sum = sum / *factor_ptr;
					scoreboard->output->at(rowIndex * ydim + colIndex) = sum;
				}
			}
		}

		template<typename T>
		void threadVectorNormalOpt4(Scoreboard<std::vector<T>>* scoreboard, size_t startIndex, size_t endIndex) {
			//std::cout << __PRETTY_FUNCTION__ << std::endl;

			// Initialise local data
			Pattern &pattern = scoreboard->pattern;
			T sum;
			int factor;
			int xdim = scoreboard->rows;
			int ydim = scoreboard->cols;
			int colLowBoundary = -scoreboard->pattern.getColumnLowerBoundary();
			int colHighBoundary = scoreboard->cols - scoreboard->pattern.getColumnHigherBoundary();
			int rowLowBoundary = -scoreboard->pattern.getRowLowerBoundary();
			int rowHighBoundary = scoreboard->rows - scoreboard->pattern.getRowHigherBoundary();
			int rowIndex = startIndex;

			// Decide margins for different column functionality with respect to rows dimensionality 
			int lower = endIndex < rowLowBoundary ? endIndex : rowLowBoundary;
			int middle = endIndex < rowHighBoundary ? endIndex : rowHighBoundary;
			int upper = endIndex < xdim ? endIndex : xdim;

			// Choose if no normalization wil be done (i.e. averaging by weight)
			int* factor_ptr = scoreboard->pattern.normalization ? &factor : &no_factor;

			// Execution
			// ------------------------------------------------------------------------------------
			// Bottom rows: all columns are affected by boundary handling
			for (; rowIndex < lower; rowIndex++) {
				for (int colIndex = 0; colIndex < ydim; colIndex++) {
					sum = 0;
					factor = 0;
					for (int offsetIndex = 0; offsetIndex < pattern.size(); offsetIndex++) {
						//std::cout << "\n";

						int i = (pattern.rowOffset(offsetIndex) + rowIndex);
						int j = (pattern.columnOffset(offsetIndex) + colIndex);

						// cut-off check
						if (i >= 0 && i < xdim && j >= 0 && j < ydim) {
							sum += scoreboard->input->at(i*ydim + j) * pattern.itemWeight(offsetIndex);
							factor += pattern.itemWeight(offsetIndex);
						}
					}
					sum = sum / *factor_ptr;
					scoreboard->output->at(rowIndex * ydim + colIndex) = sum;
				}
			}

			// Middle rows: only columns that are affected by boundary handling
			for (; rowIndex < middle; rowIndex++) {
				// Left border columns
				for (int colIndex = 0; colIndex < colLowBoundary; colIndex++) {
					factor = 0;
					sum = 0;
					for (int offsetIndex = 0; offsetIndex < pattern.size(); offsetIndex++) {
						//std::cout << "\n";

						int i = (pattern.rowOffset(offsetIndex) + rowIndex);
						int j = (pattern.columnOffset(offsetIndex) + colIndex);

						// cut-off check
						if (i >= 0 && i < xdim && j >= 0 && j < ydim) {
							sum += scoreboard->input->at(i*ydim + j) * pattern.itemWeight(offsetIndex);
							factor += pattern.itemWeight(offsetIndex);
						}
					}
					sum = sum / *factor_ptr;
					scoreboard->output->at(rowIndex * ydim + colIndex) = sum;
				}
				
				// Rgiht border columns
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
				}
			}


			// Top rows: all columns are affected by boundary handling
			for (; rowIndex < upper; rowIndex++) {
				for (int colIndex = 0; colIndex < ydim; colIndex++) {
					sum = 0;
					factor = 0;
					for (int offsetIndex = 0; offsetIndex < pattern.size(); offsetIndex++) {
						//std::cout << "\n";

						int i = (pattern.rowOffset(offsetIndex) + rowIndex);
						int j = (pattern.columnOffset(offsetIndex) + colIndex);

						// cut-off check
						if (i >= 0 && i < xdim && j >= 0 && j < ydim) {
							sum += scoreboard->input->at(i*ydim + j) * pattern.itemWeight(offsetIndex);
							factor += pattern.itemWeight(offsetIndex);
						}
					}
					sum = sum / *factor_ptr;
					scoreboard->output->at(rowIndex * ydim + colIndex) = sum;
				}
			}
		}

		// Functionality: Crop - crops out the boundry pixels (thus, no boundary checks)
		// -----------------------------------------------------------------------------

		template<typename T>
		void threadVectorCrop(Scoreboard<std::vector<T>>* scoreboard, size_t startIndex, size_t endIndex) {
			//std::cout << __PRETTY_FUNCTION__ << std::endl;

			// Initialise local data
			Pattern &pattern = scoreboard->pattern;
			T sum;
			int factor;
			int xdim = scoreboard->rows;
			int ydim = scoreboard->cols;
			int colLowBoundary = -scoreboard->pattern.getColumnLowerBoundary();
			int colHighBoundary = scoreboard->cols - scoreboard->pattern.getColumnHigherBoundary();
			int rowLowBoundary = -scoreboard->pattern.getRowLowerBoundary();
			int rowHighBoundary = scoreboard->rows - scoreboard->pattern.getRowHigherBoundary();
			// Choose if no normalization wil be done (i.e. averaging by weight)
			int* factor_ptr = scoreboard->pattern.normalization ? &factor : &no_factor;

			// Execution
			// ------------------------------------------------------------------------------------
			for (int rowIndex = startIndex; rowIndex < endIndex; rowIndex++) {
				for (int colIndex = colLowBoundary; colIndex < colHighBoundary; colIndex++) {
					sum = 0;
					factor = 0;
					for (int offsetIndex = 0; offsetIndex < pattern.size(); offsetIndex++) {
						int i = (pattern.rowOffset(offsetIndex) + rowIndex);
						int j = (pattern.columnOffset(offsetIndex) + colIndex);
						sum += scoreboard->input->at(i*ydim + j) * scoreboard->pattern.itemWeight(offsetIndex);
						factor += pattern.itemWeight(offsetIndex);
					}
					sum = sum / *factor_ptr;
					scoreboard->output->at(rowIndex * ydim + colIndex) = sum;
				}
			}
		}

		// Constructor
		// --------------------------------------------------------------------------
		StencilImplementation(size_t threads = 0) {
			nthreads = threads ? threads : std::thread::hardware_concurrency();
			THREADS = std::vector <std::thread *>(nthreads);
		}

		// Utils
		// --------------------------------------------------------------------------
		void initialiseGenerealThreadData(Pattern &pattern, const BoundaryHandling &bh, const int rows) {
			if (bh == BoundaryHandling::CROP || bh == BoundaryHandling::NORMAL_OPT_2 ) {
				num_heavier_threads = (rows - pattern.getRowHigherBoundary() + pattern.getRowLowerBoundary()) % nthreads;
				rows_per_thread = (rows - pattern.getRowHigherBoundary() + pattern.getRowLowerBoundary()) / nthreads + 1;
				index_begin = -pattern.getRowLowerBoundary();
				index_end = 0;
			}
			else {
				num_heavier_threads = (rows % nthreads);
				rows_per_thread = rows / nthreads + 1;
				index_begin = 0;
				index_end = 0;
			}
			t = 0;	
		}
	

	public:

		// Paranthesis operator for vectors: call function
		// --------------------------------------------------------------------------
		template< typename T>
		void operator()(std::vector<T> &output, std::vector<T> &input, Pattern &pattern, BoundaryHandling bh = BoundaryHandling::NORMAL, int xdim = 1, int ydim = 1) {
			auto scoreboard = new Scoreboard<std::vector<T>>(pattern, &input, &output, xdim, ydim);
			initialiseGenerealThreadData(pattern, bh, xdim);

			// Duplicative code, however, only 1 check for boundary handling
			if (bh == BoundaryHandling::NORMAL) {
				for (t = 0; t < num_heavier_threads; t++) {
					index_end = index_begin + rows_per_thread;
					THREADS.at(t) = new std::thread(&StencilImplementation::threadVectorNormal<T>, this, scoreboard, index_begin, index_end);

					index_begin += rows_per_thread;
				}
				rows_per_thread--;
				for (; t < nthreads; t++) {
					index_end = index_begin + rows_per_thread;
					THREADS.at(t) = new std::thread(&StencilImplementation::threadVectorNormal<T>, this, scoreboard, index_begin, index_end);

					index_begin += rows_per_thread;
				}
			}
			else if (bh == BoundaryHandling::WRAP) {
				for (t = 0; t < num_heavier_threads; t++) {
					index_end = index_begin + rows_per_thread;
					THREADS.at(t) = new std::thread(&StencilImplementation::threadVectorWrap<T>, this, scoreboard, index_begin, index_end);

					index_begin += rows_per_thread;
				}
				rows_per_thread--;
				for (; t < nthreads; t++) {
					index_end = index_begin + rows_per_thread;
					THREADS.at(t) = new std::thread(&StencilImplementation::threadVectorWrap<T>, this, scoreboard, index_begin, index_end);

					index_begin += rows_per_thread;
				}
			}
			else if (bh == BoundaryHandling::MIRROR) {
				for (t = 0; t < num_heavier_threads; t++) {
					index_end = index_begin + rows_per_thread;
					THREADS.at(t) = new std::thread(&StencilImplementation::threadVectorMirror<T>, this, scoreboard, index_begin, index_end);

					index_begin += rows_per_thread;
				}
				rows_per_thread--;
				for (; t < nthreads; t++) {
					index_end = index_begin + rows_per_thread;
					THREADS.at(t) = new std::thread(&StencilImplementation::threadVectorMirror<T>, this, scoreboard, index_begin, index_end);

					index_begin += rows_per_thread;
				}
			}
			else if (bh == BoundaryHandling::CROP) {
				for (t = 0; t < num_heavier_threads; t++) {
					index_end = index_begin + rows_per_thread;
					THREADS.at(t) = new std::thread(&StencilImplementation::threadVectorCrop<T>, this, scoreboard, index_begin, index_end);

					index_begin += rows_per_thread;
				}
				rows_per_thread--;
				for (; t < nthreads; t++) {
					index_end = index_begin + rows_per_thread;
					THREADS.at(t) = new std::thread(&StencilImplementation::threadVectorCrop<T>, this, scoreboard, index_begin, index_end);

					index_begin += rows_per_thread;
				}
			}
			else if (bh == BoundaryHandling::NORMAL_OPT_2) {
				size_t num_heavier_threadsBL = num_heavier_threads;
				size_t rows_per_threadBL = rows_per_thread;
				size_t index_beginBL = index_begin;
				size_t index_endBL = index_end;
				bh = BoundaryHandling::NORMAL;
				initialiseGenerealThreadData(pattern, bh, xdim);
				size_t t;
				for (t = 0; t < nthreads; t++) {
					if (num_heavier_threads == 0) { rows_per_thread--; }
					if (num_heavier_threadsBL == 0) { rows_per_threadBL--; }
					index_end = index_begin + rows_per_thread;
					index_endBL = index_beginBL + rows_per_threadBL;

					//	std::cout << index_beginBL <<" " << index_endBL-1 << " " << index_begin << " " << index_end -1  << "\n";
					THREADS.at(t) = new std::thread(&StencilImplementation::threadVectorNormalOpt2<T>, this, scoreboard, index_begin, index_end, index_beginBL, index_endBL);

					index_begin += rows_per_thread;
					index_beginBL += rows_per_threadBL;

					num_heavier_threads--;
					num_heavier_threadsBL--;
				}
			}
			else if (bh == BoundaryHandling::NORMAL_OPT_3) {
				for (t = 0; t < num_heavier_threads; t++) {
					index_end = index_begin + rows_per_thread;
					THREADS.at(t) = new std::thread(&StencilImplementation::threadVectorNormalOpt3<T>, this, scoreboard, index_begin, index_end);

					index_begin += rows_per_thread;
				}
				rows_per_thread--;
				for (; t < nthreads; t++) {
					index_end = index_begin + rows_per_thread;
					THREADS.at(t) = new std::thread(&StencilImplementation::threadVectorNormalOpt3<T>, this, scoreboard, index_begin, index_end);

					index_begin += rows_per_thread;
				}
			}
			else printf("ERROR: Given boundary handling attribute not implemented!\n");


			// Join threads
			for (t = 0; t < nthreads; ++t) { THREADS.at(t)->join(); delete THREADS[t]; }
		
		}

		// Friend Functions for Stencil Implementation Class
		// ---------------------------------------------
		friend StencilImplementation __StencilWithAccess(const size_t &threads);
	};

};

// Wrappers 
StencilSkeleton::StencilImplementation __StencilWithAccess(const size_t &threads) {
	return StencilSkeleton::StencilImplementation(threads);
}

StencilSkeleton::StencilImplementation Stencil(const size_t &threads = 0) {
	return __StencilWithAccess(threads);
}

//}
#endif // STENCIL_HPP
