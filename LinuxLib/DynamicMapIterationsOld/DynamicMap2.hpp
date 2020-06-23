#ifndef DYNAMICMAP2_HPP
#define DYNAMICMAP2_HPP

#include <cstdlib>
#include <iostream>
#include <vector>
#include <type_traits>
#include <functional>
#include <stdarg.h>
#include <typeinfo>
#include <queue>
#include <utility>
#include <thread>
#include <mutex>

class DynamicMapSkeleton2 {

private:
	DynamicMapSkeleton2() {}

	// Elemental - function used in mapping
	// ------------------------------------
	template<typename EL>
	class Elemental {
	public:
		Elemental(EL el) : elemental(el) {}
		EL elemental;
	};

public:
	// MapImplementation
	// ------------------
	template<typename EL>
	class DynamicMapImplementation2 {

	private:
		size_t nthreads;
		size_t sizeOfChunk;
		Elemental<EL> elemental;
		std::thread **allThreads;

        template<typename IN, typename OUT>
		class Scoreboard {

		public:

			// input output
			std::vector<OUT> *output;
			std::vector<IN> *input;

			// next work
			size_t curIndex = 0;
			size_t jobSize = 0;
			bool isFinished = false;

			// guard
			std::mutex lock;

			Scoreboard(std::vector<IN> *in, std::vector<OUT> *out, size_t nthreads, size_t sizeOfChunk) : output(out), input(in), jobSize(sizeOfChunk) {
			}
			~Scoreboard() {}
         

		};

		// ThreadMap - function applied to each thread
		// --------------------------------------------
		template<typename IN, typename OUT, typename ...ARGs>
		void threadMap(Scoreboard<IN,OUT> *scoreboard, size_t threadID, ARGs... args) {

			size_t elementsCount;
			size_t elementIndex;
			while (!scoreboard->isFinished) {

                    // Lock scoreboard
                    while (!scoreboard->lock.try_lock());

					if(scoreboard->isFinished) {
						scoreboard->lock.unlock();
						break;
					}

                    // get new data
                    if (scoreboard->curIndex + scoreboard->jobSize < scoreboard->input->size()){
				        elementsCount = scoreboard->jobSize;
					    elementIndex = scoreboard->curIndex;
                        scoreboard->curIndex += scoreboard->jobSize;
                    } else {
                        elementsCount = scoreboard->input->size() - scoreboard->curIndex;
                        elementIndex = scoreboard->curIndex;
                        scoreboard->curIndex += elementsCount;
						scoreboard->isFinished = true;
                    }
                    // unlock scoreboard
                    scoreboard->lock.unlock();

					// Process the data block
					// ----------------------
					for (int elementsFinished = 0; elementsFinished < elementsCount; elementsFinished++) {
						scoreboard->output->at(elementIndex+elementsFinished) = elemental.elemental(scoreboard->input->at(elementIndex+elementsFinished), args...);
					}
			}

		}

		// Constructor
		// -----------
		DynamicMapImplementation2(Elemental<EL> elemental, size_t threads, size_t sizeOfChunk) : elemental(elemental) {
			this->nthreads = threads ? threads : std::thread::hardware_concurrency();
			this->sizeOfChunk = sizeOfChunk ? sizeOfChunk : 100000;
		}

	public:
		// Paranthesis operator: call function
		// -----------------------------------
		template<typename IN, typename OUT, typename ...ARGs>
		void operator()(std::vector<OUT> &output, std::vector<IN> &input, ARGs... args) {
			this->allThreads = new std::thread*[nthreads];

			std::thread *THREADS[nthreads];
			std::vector<OUT> tempOutput(input.size());
			Scoreboard<IN, OUT> *scoreboard = new Scoreboard<IN, OUT>(&input, &output, nthreads, this->sizeOfChunk);

			// Run threads
			// -----------
			for (size_t t = 0; t < nthreads; t++) {
				allThreads[t] = new std::thread(&DynamicMapImplementation2<EL>::threadMap<IN, OUT, ARGs...>, this, ((Scoreboard<IN, OUT>*)scoreboard), t, args...);
			}

			// Join threads
			// -----------------------------------------------------------------------------------
			for (size_t t = 0; t < nthreads; ++t) {
				allThreads[t]->join();
				delete allThreads[t];
			}
			delete allThreads;
			delete ((Scoreboard<IN, OUT>*)scoreboard);
		}

		// Friend Functions for Dynamic Map Implementation Class
		// -----------------------------------------------------
		template<typename EL2>
		friend DynamicMapImplementation2<EL2> __DynamicMapWithAccess2(EL2 el, const size_t &threads, const size_t &sizeOfChunk);
	};

	// Friend Functions for Dynamic Map Skeleton Class
	// -----------------------------------------------
	template<typename EL2>
	friend DynamicMapImplementation2<EL2> __DynamicMapWithAccess2(EL2 el, const size_t &threads, const size_t &sizeOfChunk);
};

/*
* We cannot define a friend function with default argument
* that needs access to inner class on latest g++ compiler versions.
* We need a wrapper!
*/
template<typename EL>
DynamicMapSkeleton2::DynamicMapImplementation2<EL> __DynamicMapWithAccess2(EL el, const size_t &threads, const size_t &sizeOfChunk) {
	return DynamicMapSkeleton2::DynamicMapImplementation2<EL>(el, threads, sizeOfChunk);
}

template<typename EL>
DynamicMapSkeleton2::DynamicMapImplementation2<EL> DynamicMap2(EL el, const size_t &threads = 0, const size_t &sizeOfChunk = 0) {
	return __DynamicMapWithAccess2(el, threads, sizeOfChunk);
}


#endif // !SLEDMAP_H
