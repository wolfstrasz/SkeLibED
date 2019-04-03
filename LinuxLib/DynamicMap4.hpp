#ifndef DYNAMICMAP4_H
#define DYNAMICMAP4_H

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
#include <chrono>
class DynamicMapSkeleton4 {

private:
	DynamicMapSkeleton4() {}

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
	class DynamicMapImplementation4 {
	private:
		size_t nthreads;
		size_t sizeOfChunk;
		Elemental<EL> elemental;
		std::thread **allThreads;




		std::chrono::high_resolution_clock::time_point tstart;
		std::chrono::high_resolution_clock::time_point tend;
		double duration = 0.0f;
		void* scoreboard;

		template<typename IN, typename OUT>
		class Scoreboard {
		public:
			// input output
			std::vector<OUT> *output;
			std::vector<IN> *input;
			// detect global work
			bool isFinished;
			bool isInitialised;
			// detect next work
			size_t inputSize;
			size_t curIndex;
			// scoreboard worksize
			size_t jobSize;
			// guard
			std::mutex scoreboardLock;

			// timing
			// constructor
			Scoreboard(std::vector<IN> *in, std::vector<OUT> *out, size_t nthreads) {
				this->input = in;
				this->output = out;
				isFinished = false;
				isInitialised = false;
				inputSize = in->size();
				curIndex = 0;
				jobSize = 0;
			}
			~Scoreboard() {}
		};



		// ThreadMap - function applied to each thread
		// --------------------------------------------
		template<typename IN, typename OUT, typename ...ARGs>
		void threadMap(Scoreboard<IN, OUT> *scoreboard, size_t id, ARGs... args) {
			size_t elementsCount;
			size_t elementIndex;
			
			while (!scoreboard->isInitialised);


			while (!scoreboard->isFinished) {
				// Lock scoreboard
				while (!scoreboard->scoreboardLock.try_lock());
				if (scoreboard->isFinished) {
					scoreboard->scoreboardLock.unlock();
					break;
				}

				// get new data
				if (scoreboard->curIndex + scoreboard->jobSize < scoreboard->inputSize) {
					elementsCount = scoreboard->jobSize;
					elementIndex = scoreboard->curIndex;
					scoreboard->curIndex += scoreboard->jobSize;
				}
				else {
					elementsCount = scoreboard->inputSize - scoreboard->curIndex;
					elementIndex = scoreboard->curIndex;
					scoreboard->curIndex += elementsCount;
					scoreboard->isFinished = true;
				}
				// unlock scoreboard
				scoreboard->scoreboardLock.unlock();

				// Process the data block
				// ----------------------
				for (int elementsFinished = 0; elementsFinished < elementsCount; elementsFinished++) {
					scoreboard->output->at(elementIndex + elementsFinished) = elemental.elemental(scoreboard->input->at(elementIndex + elementsFinished), args...);
				}
			}

		}

		// Constructor
		// -----------
		DynamicMapImplementation4(Elemental<EL> elemental) : elemental(elemental) {
			this->nthreads = std::thread::hardware_concurrency();
			this->sizeOfChunk = sizeOfChunk ? sizeOfChunk : 100000;
			this->duration = 0.0f;
		}

	public:
		template <typename IN, typename OUT, typename ...ARGs>
		void start_init(std::vector<OUT> *output, std::vector<IN> *input, ARGs... args) {
			// create threads
			for (size_t t = 0; t < nthreads; t++) {
				allThreads[t] = new std::thread(&DynamicMapImplementation4<EL>::threadMap<IN, OUT, ARGs...>,
					this, ((Scoreboard<IN, OUT>*)scoreboard), t, args...);
			}
		}

		template <typename IN, typename OUT, typename ...ARGs>
		void start_analysis(std::vector<OUT> *output, std::vector<IN> *input, ARGs... args) {
			size_t newJobSize = 0;
			duration = 10000000.0f / nthreads; // nanosec = 0.01s	
			double durationAtStart = duration;
			// analyse worksize
			while (duration > 0.0f && newJobSize < input->size()) {
				tstart = std::chrono::high_resolution_clock::now();
				output->at(newJobSize) = elemental.elemental(input->at(newJobSize), args...);
				tend = std::chrono::high_resolution_clock::now();
				duration -= (double)std::chrono::duration_cast<std::chrono::nanoseconds>(tend - tstart).count();
				newJobSize++;
			}
			((Scoreboard<IN, OUT>*)scoreboard)->curIndex = newJobSize;
			newJobSize *= nthreads;

			// send work size
			((Scoreboard<IN, OUT>*)scoreboard)->jobSize = newJobSize;
			((Scoreboard<IN, OUT>*)scoreboard)->isInitialised = true;
		}


		// Paranthesis operator: call function
		// -----------------------------------
		template<typename IN, typename OUT, typename ...ARGs>
		void operator()(std::vector<OUT> &output, std::vector<IN> &input, ARGs... args) {
			this->allThreads = new std::thread*[nthreads];
			scoreboard = new Scoreboard<IN, OUT>(&input, &output, nthreads);

			// USE THREADER
			// -----------------------------------------------------------------------------------
			// start threader
			std::thread *threader;
			threader = new std::thread(&DynamicMapImplementation4<EL>::start_init<IN, OUT, ARGs...>, this, &output, &input, args...);

			// main thread analyses
			start_analysis(&output, &input, args...);


			// delete threader
			threader->join();
			delete threader;

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
	friend DynamicMapImplementation4<EL2> __DynamicMapWithAccess4(EL2 el, const size_t &threads, const size_t &sizeOfChunk);
	};

	// Friend Functions for Dynamic Map Skeleton Class
	// -----------------------------------------------
	template<typename EL2>
	friend DynamicMapImplementation4<EL2> __DynamicMapWithAccess4(EL2 el, const size_t &threads, const size_t &sizeOfChunk);
};

/*
* We cannot define a friend function with default argument
* that needs access to inner class on latest g++ compiler versions.
* We need a wrapper!
*/
template<typename EL>
DynamicMapSkeleton4::DynamicMapImplementation4<EL> __DynamicMapWithAccess4(EL el, const size_t &threads, const size_t &sizeOfChunk) {
	return DynamicMapSkeleton4::DynamicMapImplementation4<EL>(el, threads, sizeOfChunk);
}

template<typename EL>
DynamicMapSkeleton4::DynamicMapImplementation4<EL> DynamicMap4(EL el, const size_t &threads = 0, const size_t &sizeOfChunk = 0) {
	return __DynamicMapWithAccess4(el, threads, sizeOfChunk);
}



#endif // !SLEDMAP_H
