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

#define PSLED_WRAP StencilSkeleton::BoundaryHandling::WRAP
#define PSLED_NORMAL StencilSkeleton::BoundaryHandling::NORMAL
#define PSLED_MIRROR StencilSkeleton::BoundaryHandling::MIRROR
#define PSLED_CROP StencilSkeleton::BoundaryHandling::CROP
#define PSLED_OPT_NORMAL StencilSkeleton::BoundaryHandling::OPT_NORMAL

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
		OPT_NORMAL
	};

	// StencilImplementation
	// ---------------------
	class StencilImplementation {
	private:

		size_t nthreads;
		std::vector<std::thread *> THREADS;
		
		// Utility class for easier data sending - creates some overhead
		// ---------------------------------------------------------------------------------
		template <typename T>
		class Stencilboard {
		public:
			Pattern pattern;
			int rows;
			int cols;
			std::vector<T>* input;
			std::vector<T>* output;
			Stencilboard(Pattern &pattern, std::vector<T> * input, std::vector<T> * output, int rows = 1, int cols = 1) 
			: pattern (pattern), input (input), output (output), rows(rows), cols(cols) {}
		};

		// Constructor
		// ---------------------------------------------------------------------------------
		StencilImplementation(size_t threads = 0) {
			nthreads = threads ? threads : std::thread::hardware_concurrency();
			THREADS = std::vector <std::thread *>(nthreads);
		}

		// Wrap Boundary handling - Uses wrapping around boundaries
		// ---------------------------------------------------------------------------------
		template<typename T>
		void threadWrap(Stencilboard<T>* stencilboard, size_t startIndex, size_t endIndex) {
			// Store data to reduce dereferencing calls
			Pattern &pattern = stencilboard->pattern;
			int xdim = stencilboard->rows;
			int ydim = stencilboard->cols;

			// Data to calculate the kernel
			T sum;
			int factor;
			int no_factor = 1;

			// Choose if no normalization wil be done (i.e. averaging by weight)
			int* factor_ptr = pattern.normalization ? &factor : &no_factor;

			// Execution
			for (int rowIndex = startIndex; rowIndex < endIndex; rowIndex++) {
				for (int colIndex = 0; colIndex < ydim; colIndex++) {
					sum = 0;
					factor = 0;
					for (int offsetIndex = 0; offsetIndex < pattern.size(); offsetIndex++) {
						int i = (pattern.rowOffset(offsetIndex) + rowIndex);
						int j = (pattern.columnOffset(offsetIndex) + colIndex);

						// wrapping
						i = (i + xdim) % xdim;
						j = (j + ydim) % ydim;

						sum += stencilboard->input->at(i*ydim + j)* pattern.itemWeight(offsetIndex);
						factor += pattern.itemWeight(offsetIndex);
					}
					sum = sum / *factor_ptr;
					stencilboard->output->at(rowIndex * ydim + colIndex) = sum;
				}
			}
		}

		// Mirrored boundary handling - Uses mirrored values when handling boundaries
		// ---------------------------------------------------------------------------------
		template<typename T>
		void threadMirror(Stencilboard<T>* stencilboard, size_t startIndex, size_t endIndex) {
			// Store data to reduce dereferencing calls			
			Pattern &pattern = stencilboard->pattern;
			int xdim = stencilboard->rows;
			int ydim = stencilboard->cols;

			// Data to calculate the kernel
			T sum;
			int no_factor = 1;
			int factor;

			// Choose if no normalization wil be done (i.e. averaging by weight)
			int* factor_ptr = pattern.normalization ? &factor : &no_factor;

			// Execution
			for (int rowIndex = startIndex; rowIndex < endIndex; rowIndex++) {
				for (int colIndex = 0; colIndex < ydim; colIndex++) {
					sum = 0;
					factor = 0;
					for (int offsetIndex = 0; offsetIndex < pattern.size(); offsetIndex++) {
						int i = (pattern.rowOffset(offsetIndex) + rowIndex);
						int j = (pattern.columnOffset(offsetIndex) + colIndex);
						// mirroring
						if (i < 0 || i >= xdim) i = (rowIndex - pattern.rowOffset(offsetIndex));
						if (j < 0 || j >= ydim) j = (colIndex - pattern.columnOffset(offsetIndex));

						sum += stencilboard->input->at(i*ydim + j)* pattern.itemWeight(offsetIndex);
						factor += pattern.itemWeight(offsetIndex);
					}
					sum = sum / *factor_ptr;
					stencilboard->output->at(rowIndex * ydim + colIndex) = sum;
				}
			}
		}

		// Normal Boundary handling - Does not use other values to exchange unobtainable offset items (cut-off)
		// ---------------------------------------------------------------------------------
		template<typename T>
		void threadNormal(Stencilboard<T>* stencilboard, size_t startIndex, size_t endIndex) {
			// Store data to reduce dereferencing calls			
			Pattern &pattern = stencilboard->pattern;
			int xdim = stencilboard->rows;
			int ydim = stencilboard->cols;

			// Data to calculate the kernel
			T sum;
			int no_factor = 1;
			int factor;

			// Choose if no normalization wil be done (i.e. averaging by weight)
			int* factor_ptr = pattern.normalization ? &factor : &no_factor;

			// Execution
			for (int rowIndex = startIndex; rowIndex < endIndex; rowIndex++) {
				for (int colIndex = 0; colIndex < stencilboard->cols; colIndex++) {
					sum = 0;
					factor = 0;
					for (int offsetIndex = 0; offsetIndex < pattern.size(); offsetIndex++) {

						int i = (pattern.rowOffset(offsetIndex) + rowIndex);
						int j = (pattern.columnOffset(offsetIndex) + colIndex);

						// cut-off check
						if (i >= 0 && i < xdim && j >= 0 && j < ydim) {
							sum += stencilboard->input->at(i*ydim + j) * pattern.itemWeight(offsetIndex);
							factor += pattern.itemWeight(offsetIndex);
						}
					}
					sum = sum / *factor_ptr;
					stencilboard->output->at(rowIndex * ydim + colIndex) = sum;
				}
			}
		}

		// Crop Boundary handling - does no working on values near the border
		// -----------------------------------------------------------------------------
		template<typename T>
		void threadCrop(Stencilboard<T>* stencilboard, size_t startIndex, size_t endIndex) {
			// Store data to reduce dereferencing calls			
			Pattern &pattern = stencilboard->pattern;
			int ydim = stencilboard->cols;

			// Data to calculate the kernel
			T sum;
			int no_factor = 1;
			int factor;

			// Choose if no normalization wil be done (i.e. averaging by weight)
			int* factor_ptr = pattern.normalization ? &factor : &no_factor;

			// Used in cropping the colums
			int colLowBoundary = -pattern.getColumnLowerBoundary();
			int colHighBoundary = ydim - pattern.getColumnHigherBoundary();

			// Execution
			for (int rowIndex = startIndex; rowIndex < endIndex; rowIndex++) {
				for (int colIndex = colLowBoundary; colIndex < colHighBoundary; colIndex++) {
					sum = 0;
					factor = 0;
					for (int offsetIndex = 0; offsetIndex < pattern.size(); offsetIndex++) {
						int i = (pattern.rowOffset(offsetIndex) + rowIndex);
						int j = (pattern.columnOffset(offsetIndex) + colIndex);
						sum += stencilboard->input->at(i*ydim + j) * stencilboard->pattern.itemWeight(offsetIndex);
						factor += pattern.itemWeight(offsetIndex);
					}
					sum = sum / *factor_ptr;
					stencilboard->output->at(rowIndex * ydim + colIndex) = sum;
				}
			}
		}


		// Normal Boundary handling where code is unfolded to minimize branching overhead from checks
		// ---------------------------------------------------------------------------------
		template<typename T>
		void threadOptimisedNormal(Stencilboard<T>* stencilboard, size_t startIndex, size_t endIndex) {
			// Store data to reduce dereferencing calls			
			Pattern &pattern = stencilboard->pattern;
			int xdim = stencilboard->rows;
			int ydim = stencilboard->cols;

			// Data to calculate the kernel
			T sum;
			int no_factor = 1;
			int factor;

			// Choose if no normalization wil be done (i.e. averaging by weight)
			int* factor_ptr = pattern.normalization ? &factor : &no_factor;
			
			// Determine the boundaries that will split the input into 9 chunks
			int colLowBoundary = -pattern.getColumnLowerBoundary();
			int colHighBoundary = ydim - pattern.getColumnHigherBoundary();
			int rowLowBoundary = -stencilboard->pattern.getRowLowerBoundary();
			int rowHighBoundary = xdim - pattern.getRowHigherBoundary();
			
			// Decide margins of chunks with respect to rows task separation and boundaries
			int lower = endIndex < rowLowBoundary ? endIndex : rowLowBoundary;
			int middle = endIndex < rowHighBoundary ? endIndex : rowHighBoundary;
			int upper = endIndex < xdim ? endIndex : xdim;


			// Execution
			int rowIndex = startIndex;

			// Lower rows: all columns are affected by boundary handling
			for (; rowIndex < lower; rowIndex++) {
				// Left border column boundary handling
				for (int colIndex = 0; colIndex < colLowBoundary; colIndex++) {
					factor = 0;
					sum = 0;
					for (int offsetIndex = 0; offsetIndex < pattern.size(); offsetIndex++) {
						int i = (pattern.rowOffset(offsetIndex) + rowIndex);
						int j = (pattern.columnOffset(offsetIndex) + colIndex);

						// Boundary check
						if (i >= 0 && j >= 0) {
							sum += stencilboard->input->at(i*ydim + j)* pattern.itemWeight(offsetIndex);
							factor += pattern.itemWeight(offsetIndex);
						}
					}
					sum = sum / *factor_ptr;
					stencilboard->output->at(rowIndex * ydim + colIndex) = sum;
				}
				
				// Middle columns only affected by row boundary
				for (int colIndex = colLowBoundary; colIndex < colHighBoundary; colIndex++) {
					sum = 0;
					factor = 0;
					for (int offsetIndex = 0; offsetIndex < pattern.size(); offsetIndex++) {
						int i = (pattern.rowOffset(offsetIndex) + rowIndex);
						int j = (pattern.columnOffset(offsetIndex) + colIndex);

						// Boundary check
						if (i >= 0){
							sum += stencilboard->input->at(i*ydim + j) * pattern.itemWeight(offsetIndex);
							factor += pattern.itemWeight(offsetIndex);
						}
					}
					sum = sum / *factor_ptr;
					stencilboard->output->at(rowIndex * ydim + colIndex) = sum;
				}

				// Right border column boundary handling
				for (int colIndex = colHighBoundary; colIndex < ydim; colIndex++) {
					sum = 0;
					factor = 0;
					for (int offsetIndex = 0; offsetIndex < pattern.size(); offsetIndex++) {

						int i = (pattern.rowOffset(offsetIndex) + rowIndex);
						int j = (pattern.columnOffset(offsetIndex) + colIndex);

						// Boundary check
						if (i >=0 && j < ydim) {
							sum += stencilboard->input->at(i*ydim + j)* pattern.itemWeight(offsetIndex);
							factor += pattern.itemWeight(offsetIndex);
						}
					}
					sum = sum / *factor_ptr;
					stencilboard->output->at(rowIndex * ydim + colIndex) = sum;
				}
			}
			
			// Middle rows not affected by row boundaries
			for (; rowIndex < middle; rowIndex++) {

				// Left border column boundary handling
				for (int colIndex = 0; colIndex < colLowBoundary; colIndex++) {
					factor = 0;
					sum = 0;
					for (int offsetIndex = 0; offsetIndex < pattern.size(); offsetIndex++) {
						int i = (pattern.rowOffset(offsetIndex) + rowIndex);
						int j = (pattern.columnOffset(offsetIndex) + colIndex);

						// Boundary check
						if (j >= 0) {
							sum += stencilboard->input->at(i*ydim + j)* pattern.itemWeight(offsetIndex);
							factor += pattern.itemWeight(offsetIndex);
						}
					}
					sum = sum / *factor_ptr;
					stencilboard->output->at(rowIndex * ydim + colIndex) = sum;
				}
				
				// Middle columns not affected by any boundary handling
				for (int colIndex = colLowBoundary; colIndex < colHighBoundary; colIndex++) {
					sum = 0;
					factor = 0;
					for (int offsetIndex = 0; offsetIndex < pattern.size(); offsetIndex++) {
						int i = (pattern.rowOffset(offsetIndex) + rowIndex);
						int j = (pattern.columnOffset(offsetIndex) + colIndex);

						sum += stencilboard->input->at(i*ydim + j) * pattern.itemWeight(offsetIndex);
						factor += pattern.itemWeight(offsetIndex);
					}
					sum = sum / *factor_ptr;
					stencilboard->output->at(rowIndex * ydim + colIndex) = sum;
				}

				// Right border column boundary handling
				for (int colIndex = colHighBoundary; colIndex < stencilboard->cols; colIndex++) {
					sum = 0;
					factor = 0;
					for (int offsetIndex = 0; offsetIndex < pattern.size(); offsetIndex++) {

						int i = (pattern.rowOffset(offsetIndex) + rowIndex);
						int j = (pattern.columnOffset(offsetIndex) + colIndex);

						// Boundary check
						if (j < ydim) {
							sum += stencilboard->input->at(i*ydim + j)* pattern.itemWeight(offsetIndex);
							factor += pattern.itemWeight(offsetIndex);
						}
					}
					sum = sum / *factor_ptr;
					stencilboard->output->at(rowIndex * ydim + colIndex) = sum;
				}
			}
		
			// Upper rows: all columns are affected by boundary handling
			for (; rowIndex < upper; rowIndex++) {
				// Left border column boundary handling
				for (int colIndex = 0; colIndex < colLowBoundary; colIndex++) {
					factor = 0;
					sum = 0;
					for (int offsetIndex = 0; offsetIndex < pattern.size(); offsetIndex++) {
						int i = (pattern.rowOffset(offsetIndex) + rowIndex);
						int j = (pattern.columnOffset(offsetIndex) + colIndex);

						// Boundary check
						if (i < xdim && j >= 0) {
							sum += stencilboard->input->at(i*ydim + j)* pattern.itemWeight(offsetIndex);
							factor += pattern.itemWeight(offsetIndex);
						}
					}
					sum = sum / *factor_ptr;
					stencilboard->output->at(rowIndex * ydim + colIndex) = sum;
				}
				
				// Middle columns only affected by row boundary
				for (int colIndex = colLowBoundary; colIndex < colHighBoundary; colIndex++) {
					sum = 0;
					factor = 0;
					for (int offsetIndex = 0; offsetIndex < pattern.size(); offsetIndex++) {
						int i = (pattern.rowOffset(offsetIndex) + rowIndex);
						int j = (pattern.columnOffset(offsetIndex) + colIndex);
						if (i < xdim){
							sum += stencilboard->input->at(i*ydim + j) * pattern.itemWeight(offsetIndex);
							factor += pattern.itemWeight(offsetIndex);
						}
					}
					sum = sum / *factor_ptr;
					stencilboard->output->at(rowIndex * ydim + colIndex) = sum;
				}

				// Right border column boundary handling
				for (int colIndex = colHighBoundary; colIndex < stencilboard->cols; colIndex++) {
					sum = 0;
					factor = 0;
					for (int offsetIndex = 0; offsetIndex < pattern.size(); offsetIndex++) {

						int i = (pattern.rowOffset(offsetIndex) + rowIndex);
						int j = (pattern.columnOffset(offsetIndex) + colIndex);

						// Boundary check
						if ( i < xdim && j < ydim) {
							sum += stencilboard->input->at(i*ydim + j)* pattern.itemWeight(offsetIndex);
							factor += pattern.itemWeight(offsetIndex);
						}
					}
					sum = sum / *factor_ptr;
					stencilboard->output->at(rowIndex * ydim + colIndex) = sum;
				}
			}
		}


	public:
	
		// User call function
		// --------------------------------------------------------------------------
		template< typename T>
		void operator()(std::vector<T> &output, std::vector<T> &input, Pattern &pattern,
					 BoundaryHandling bh = BoundaryHandling::NORMAL, int xdim = 1, int ydim = 1) {
			auto stencilboard = new Stencilboard<T>(pattern, &input, &output, xdim, ydim);

			// if rows of input cannot be split equally amongst threads
			// calculate the number of threads that will have more rows
			int num_heavier_threads = (xdim % nthreads);
			int rows_per_thread = xdim / nthreads + 1;
			int index_begin = 0;
			int index_end = 0;
			auto bhFunctor = &StencilImplementation::threadNormal<T>;

			// Get the boundary handling function 
			switch (bh){
				case BoundaryHandling::NORMAL : 
					bhFunctor = &StencilImplementation::threadNormal<T>;
					break;
				case BoundaryHandling::WRAP :
					bhFunctor = &StencilImplementation::threadWrap<T>;
					break;
				case BoundaryHandling::MIRROR :
					bhFunctor = &StencilImplementation::threadMirror<T>;
					break;
				case BoundaryHandling::OPT_NORMAL :
					bhFunctor = &StencilImplementation::threadOptimisedNormal<T>;
					break;
				case BoundaryHandling::CROP :
					// For the crop we need to not assign to the execution the top and bottom rows (they are cropped)
					bhFunctor = &StencilImplementation::threadCrop<T>;
					num_heavier_threads = (xdim - pattern.getRowHigherBoundary() + pattern.getRowLowerBoundary()) % nthreads;
					rows_per_thread = (xdim - pattern.getRowHigherBoundary() + pattern.getRowLowerBoundary()) / nthreads + 1;
					index_begin = -pattern.getRowLowerBoundary();
					break;
				default :
					printf("ERROR: Given boundary handling attribute not implemented!\n");
					return ;
			}

			int t = 0;
			// First threads will have 1 more row for work
			for (; t < num_heavier_threads; ++t) {
				index_end = index_begin + rows_per_thread;
				THREADS.at(t) = new std::thread(bhFunctor, this, stencilboard, index_begin, index_end);

				index_begin += rows_per_thread;
			}
			
			rows_per_thread--;
			// latter if any will have less rows to work on
			for (; t < nthreads; ++t) {
				index_end = index_begin + rows_per_thread;
				THREADS.at(t) = new std::thread(bhFunctor, this, stencilboard, index_begin, index_end);

				index_begin += rows_per_thread;
			}

			// Join threads
			for (int t = 0; t < nthreads; ++t) { THREADS.at(t)->join(); delete THREADS[t]; }
			delete stencilboard;
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

#endif // STENCIL_HPP
