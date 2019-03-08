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
		BORDER
	};


	// StencilImplementation
	// ---------------------
	class StencilImplementation {
	private:

		size_t nthreads;
		template <typename T>
		class Scoreboard {
		public:
			int rows, cols;
			T* input;
			T* output;
			Scoreboard(T *input, T *output, int rows = 0, int cols = 0) {
				this->input = input;
				this->output = output;
				this->rows = rows;
				this->cols = cols;
			}

		};
		void * scoreboard;


		// ThreadWrap - function applied to each thread
		// Uses boundary wrapping
		// --------------------------------------------
		template<typename T>
		void threadWrap(Scoreboard<T>* scoreboard, Pattern so, size_t startIndex, size_t endIndex) {
			T sum;
			for (int rowIndex = startIndex; rowIndex < endIndex; rowIndex++) {
				for (int colIndex = 0; colIndex < scoreboard->cols; colIndex++) {

					int factor = 0;
					for (int offsetIndex = 0; offsetIndex < so.size(); offsetIndex++) {
						int i = (so.rowOffset(offsetIndex) + rowIndex);
						int j = (so.columnOffset(offsetIndex) + colIndex);
						i = (i + scoreboard->rows) % scoreboard->rows;
						j = (j + scoreboard->cols) % scoreboard->cols;
						sum += (*(scoreboard->input + i * scoreboard->rows + j)) * so.itemWeight(offsetIndex);
						factor += so.itemWeight(offsetIndex);
					}

					sum = sum / factor;
					*(scoreboard->output + rowIndex * scoreboard->rows + colIndex) = sum;
					sum -= sum;
				}
			}

		}

		// ThreadNormal - function applied to each thread
		// Does not use other values to exchange unobtainable offset items
		// ---------------------------------------------------------------
		template<typename T>
		void threadNormal(Scoreboard<T>* scoreboard, Pattern so, size_t startIndex, size_t endIndex) {
			T sum;
			for (int rowIndex = startIndex; rowIndex < endIndex; rowIndex++) {
				for (int colIndex = 0; colIndex < scoreboard->cols; colIndex++) {
					int factor = 0;
					for (int offsetIndex = 0; offsetIndex < so.size(); offsetIndex++) {
						int i = (so.rowOffset(offsetIndex) + rowIndex);
						int j = (so.columnOffset(offsetIndex) + colIndex);
						if (i >= 0 && i < scoreboard->rows && j >= 0 && j < scoreboard->cols) {
							sum += (*(scoreboard->input + i * scoreboard->rows + j)) * so.itemWeight(offsetIndex);
							factor += so.itemWeight(offsetIndex);

						}
					}

					sum = sum / factor;
					*(scoreboard->output + rowIndex * scoreboard->rows + colIndex) = sum;
					sum -= sum;
				}
			}
		}

		// ThreadMirror - function applied to each thread
		// Uses mirrored values when handling boundaries
		// ---------------------------------------------
		template<typename T>
		void threadMirror(Scoreboard<T>* scoreboard, Pattern so, size_t startIndex, size_t endIndex) {
			T sum;
			for (int rowIndex = startIndex; rowIndex < endIndex; rowIndex++) {
				for (int colIndex = 0; colIndex < scoreboard->cols; colIndex++) {

					int factor = 0;
					for (int offsetIndex = 0; offsetIndex < so.size(); offsetIndex++) {
						int i = (so.rowOffset(offsetIndex) + rowIndex);
						int j = (so.columnOffset(offsetIndex) + colIndex);
						if (i < 0 || i >= scoreboard->rows) i = (rowIndex - so.rowOffset(offsetIndex));
						if (j < 0 || j >= scoreboard->cols) j = (colIndex - so.columnOffset(offsetIndex));
						sum += (*(scoreboard->input + i * scoreboard->rows + j)) * so.itemWeight(offsetIndex);
						factor += so.itemWeight(offsetIndex);
					}

					sum = sum / factor;
					*(scoreboard->output + rowIndex * scoreboard->rows + colIndex) = sum;
					sum -= sum;
				}
			}
		}

		// ThreadBorder - function applied to each thread
		// Uses offsets' size margins to calculate untouched area (border)
		// ---------------------------------------------------------------
		template<typename T>
		void threadBorder(Scoreboard<T>* scoreboard, Pattern so, size_t startIndex, size_t endIndex) {
			T sum;
			for (int rowIndex = startIndex; rowIndex < endIndex; rowIndex++) {
				for (int colIndex = -so.getColumnLowerBoundary(); colIndex < scoreboard->cols- so.getColumnHigherBoundary(); colIndex++) {
					int factor = 0;
					for (int offsetIndex = 0; offsetIndex < so.size(); offsetIndex++) {
						int i = (so.rowOffset(offsetIndex) + rowIndex);
						int j = (so.columnOffset(offsetIndex) + colIndex);
						sum += (*(scoreboard->input + i * scoreboard->rows + j)) * so.itemWeight(offsetIndex);
						factor += so.itemWeight(offsetIndex);
					}

					sum = sum / factor;
					*(scoreboard->output + rowIndex * scoreboard->rows + colIndex) = sum;
					sum -= sum;
				}
			}
		}

		// Constructor
		// -----------
		StencilImplementation(size_t threads = 0)  {
			nthreads = threads ? threads : std::thread::hardware_concurrency();

		}

		// VECTOR BORDER
		template<typename T>
		void threadVectorBorder(Scoreboard<std::vector<T>>* scoreboard, Pattern so, size_t startIndex, size_t endIndex) {
			//std::cout << __PRETTY_FUNCTION__ << std::endl;
			//std::cout << "HELLO\n" << startIndex << " "<< endIndex << std::endl;
			T sum;
			sum = 0;
			//std::cout << sum << std::endl;
			int xdim = scoreboard->rows;
			int ydim = scoreboard->cols;
			//std::cout << "DIMENSION = (" << xdim << ", " << ydim << ")\n";
			//std::cout << "ROW INDEX = (" << startIndex << ", " << endIndex << ")\n";

			//std::cout << "so. size  = " << so.size()<<std::endl;
		
			for (int rowIndex = startIndex; rowIndex < endIndex; rowIndex++) {
				for (int colIndex = -so.getColumnLowerBoundary(); colIndex < scoreboard->cols - so.getColumnHigherBoundary(); colIndex++) {

			//		std::cout << "for (" << rowIndex << ", " << colIndex << ")\n";
					int factor = 0;
					for (int offsetIndex = 0; offsetIndex < so.size(); offsetIndex++) {
						int i = (so.rowOffset(offsetIndex) + rowIndex);
						int j = (so.columnOffset(offsetIndex) + colIndex);
						sum += scoreboard->input->at(i*xdim + j);
						factor += so.itemWeight(offsetIndex);
					}
			//		std::cout << "we have: " << sum << "(sum), " << factor << "(factor)\n";
					sum = sum / factor;
			//		std::cout << "ending in sum: " << sum << std::endl;
					scoreboard->output->at(rowIndex * xdim + colIndex) = sum;
					sum -= sum;
				}
			}
		}

	public:
		// Paranthesis operator: call function
		// -----------------------------------
		template<size_t rows, size_t cols, typename T>
		void operator()(T(&output)[rows][cols], T(&input)[rows][cols], Pattern so, BoundaryHandling bh = BoundaryHandling::NORMAL) {
			// std::cout << __PRETTY_FUNCTION__ << std::endl;
			// std::cout<< "Stencil run with " << nthreads << " threads!\n" ;

			std::thread *THREADS[nthreads];
			scoreboard = new Scoreboard<T>((T*)input, (T*)output, rows, cols);
		//		std::cout << "rows = " << rows << " nthreads = " << nthreads << " rows \% nthreads = " << rows % nthreads << "\n";
			size_t t = 0;
			size_t higher_threads = rows % nthreads;
			size_t rows_per_thread = rows  / nthreads;
			size_t rowStartIndex = 0 ;
			size_t rowEndIndex = 0;
			// Run threads
			// -----------
			if (bh == BoundaryHandling::WRAP) {

				for (t = 0; t < higher_threads; t++) {
					rowEndIndex = rowStartIndex + rows_per_thread + 1;
					THREADS[t] = new std::thread(&StencilImplementation::threadWrap<T>, this, (Scoreboard<T>*)scoreboard, so, rowStartIndex, rowEndIndex);
					rowStartIndex += rows_per_thread + 1;
				}

				for (;t<nthreads; t++){
					rowEndIndex = rowStartIndex + rows_per_thread;
					THREADS[t] = new std::thread(&StencilImplementation::threadWrap<T>, this, (Scoreboard<T>*)scoreboard, so, rowStartIndex, rowEndIndex);
					rowStartIndex += rows_per_thread;
				}
			}
			else if (bh == BoundaryHandling::MIRROR) {
				// CAN OPTIMIZE IN TWO RUNS for boundary ones and non
				for (t = 0; t < higher_threads; t++) {
					rowEndIndex = rowStartIndex + rows_per_thread + 1;
					THREADS[t] = new std::thread(&StencilImplementation::threadMirror<T>, this, (Scoreboard<T>*)scoreboard, so, rowStartIndex, rowEndIndex);
					rowStartIndex += rows_per_thread + 1;
				}

				for (;t<nthreads; t++){
					rowEndIndex = rowStartIndex + rows_per_thread;
					THREADS[t] = new std::thread(&StencilImplementation::threadMirror<T>, this, (Scoreboard<T>*)scoreboard, so, rowStartIndex, rowEndIndex);
					rowStartIndex += rows_per_thread;
				}
			}
			else if (bh == BoundaryHandling::BORDER) {
					size_t higher_threads = (rows - so.getRowHigherBoundary() + so.getRowLowerBoundary()) % nthreads;
					size_t rows_per_thread = (rows - so.getRowHigherBoundary() + so.getRowLowerBoundary()) / nthreads;
					size_t rowStartIndex = -so.getRowLowerBoundary();
					size_t rowEndIndex = 0;
					for (t = 0; t < higher_threads; t++) {
						rowEndIndex = rowStartIndex + rows_per_thread + 1;
						THREADS[t] = new std::thread(&StencilImplementation::threadBorder<T>, this, (Scoreboard<T>*)scoreboard, so, rowStartIndex, rowEndIndex);
					//	 std::cout << "(" << rowStartIndex << ", " << rowEndIndex << ")\n";

						rowStartIndex += rows_per_thread + 1;
					}

					for (;t<nthreads; t++){
						rowEndIndex = rowStartIndex + rows_per_thread;
						THREADS[t] = new std::thread(&StencilImplementation::threadBorder<T>, this, (Scoreboard<T>*)scoreboard, so, rowStartIndex, rowEndIndex);
						// std::cout << "(" << rowStartIndex << ", " << rowEndIndex << ")\n";

						rowStartIndex += rows_per_thread;
					}
			}
			else { // NORMAL BOUNDARY HANDLING
				// CAN OPTIMIZE IN TWO RUNS for boundary ones and non
				for (t = 0; t < higher_threads; t++) {
					rowEndIndex = rowStartIndex + rows_per_thread + 1;
					THREADS[t] = new std::thread(&StencilImplementation::threadNormal<T>, this, (Scoreboard<T>*)scoreboard, so, rowStartIndex, rowEndIndex);
					rowStartIndex += rows_per_thread + 1;
				}

				for (;t<nthreads; t++){
					rowEndIndex = rowStartIndex + rows_per_thread;
					THREADS[t] = new std::thread(&StencilImplementation::threadNormal<T>, this, (Scoreboard<T>*)scoreboard, so, rowStartIndex, rowEndIndex);
					rowStartIndex += rows_per_thread;
				}
			}

			// Join threads
			// ------------
			for (size_t t = 0; t < nthreads; ++t) { THREADS[t]->join(); delete THREADS[t]; }

		}
		
		template< typename T>
		void operator()(std::vector<T> &output, std::vector<T> &input, Pattern so, BoundaryHandling bg = BoundaryHandling::NORMAL, int xdim = 0, int ydim = 0) {
			std::thread *THREADS[nthreads];

			scoreboard = new Scoreboard<std::vector<T>>(&input, &output, xdim, ydim);
			//std::cout << "(" << sb->rows << ", " << sb->cols << ")\n";
			/*for (int i = 0; i < sb->rows; i++) {
				
				for (int j = 0; j < sb->cols; j++) {
					std::cout << (sb->input)->at(i*xdim + j) << " ";
				}
				std::cout << std::endl;
			}*/

			// USE BORDER
			size_t t = 0;
			size_t higher_threads = (xdim - so.getRowHigherBoundary() + so.getRowLowerBoundary()) % nthreads;
			size_t rows_per_thread = (xdim - so.getRowHigherBoundary() + so.getRowLowerBoundary()) / nthreads;
			size_t rowStartIndex = -so.getRowLowerBoundary();
			size_t rowEndIndex = 0;

			//std::cout << "use border\n";
			for (t = 0; t < higher_threads; t++) {
				rowEndIndex = rowStartIndex + rows_per_thread + 1;
				THREADS[t] = new std::thread(&StencilImplementation::threadVectorBorder<T>, this, (Scoreboard<std::vector<T>>*)scoreboard, so, rowStartIndex, rowEndIndex);
				//	 std::cout << "f1 -> (" << rowStartIndex << ", " << rowEndIndex << ")\n";

				rowStartIndex += rows_per_thread + 1;
			}

			for (; t < nthreads; t++) {
				rowEndIndex = rowStartIndex + rows_per_thread;
				THREADS[t] = new std::thread(&StencilImplementation::threadVectorBorder<T>, this, (Scoreboard<std::vector<T>>*)scoreboard, so, rowStartIndex, rowEndIndex);
				// std::cout << "f2 -> (" << rowStartIndex << ", " << rowEndIndex << ")\n";

				rowStartIndex += rows_per_thread;
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
