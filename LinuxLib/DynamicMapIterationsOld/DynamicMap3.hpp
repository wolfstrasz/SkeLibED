#ifndef DYNAMICMAP3_HPP
#define DYNAMICMAP3_HPP

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
class DynamicMapSkeleton3 {

private:
	DynamicMapSkeleton3() {}

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
	class DynamicMapImplementation3 {
	private:
		size_t nthreads;
		size_t sizeOfChunk;
		Elemental<EL> elemental;
		std::thread **allThreads;




		/* SPECIALIZED */
		std::chrono::high_resolution_clock::time_point tstart;
		std::chrono::high_resolution_clock::time_point tend;
		double duration = 0.0f;
		/////////////////////////


		template<typename IN, typename OUT>
		class Scoreboard {
		public:
			// input output
			std::vector<OUT> *output;
			std::vector<IN> *input;
			// detect next work
			size_t jobSize = 0;;
			size_t curIndex = 0;
			bool isFinished = false;

			// guard
			std::mutex lock;



			// detect global work
			bool isInitialised = false;

			// constructor
			Scoreboard(std::vector<IN> *in, std::vector<OUT> *out, size_t nthreads, size_t sizeOfChunk) : output(out), input(in), jobSize(sizeOfChunk) {}
			~Scoreboard() {}
		};


		// ThreadMap - function applied to each thread
		// --------------------------------------------
		template<typename IN, typename OUT, typename ...ARGs>
		void threadMap(Scoreboard<IN, OUT> *scoreboard,size_t id, ARGs... args) {
			size_t elementsCount;
			size_t elementIndex;

			while (!scoreboard->isInitialised);

			while (!scoreboard->isFinished) {
				// Lock scoreboard
				while (!scoreboard->lock.try_lock());
				if (scoreboard->isFinished) {
					scoreboard->lock.unlock();
					break;
				}

				// get new data
				if (scoreboard->curIndex + scoreboard->jobSize < scoreboard->input->size()) {
					elementsCount = scoreboard->jobSize;
					elementIndex = scoreboard->curIndex;
					scoreboard->curIndex += scoreboard->jobSize;
				}
				else {
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
					scoreboard->output->at(elementIndex + elementsFinished) = elemental.elemental(scoreboard->input->at(elementIndex + elementsFinished), args...);
				}
			}

		}
		// Constructor
		// -----------
		DynamicMapImplementation3(Elemental<EL> elemental, size_t threads, size_t sizeOfChunk) : elemental(elemental) {
			this->nthreads = threads ? threads : std::thread::hardware_concurrency();
			this->sizeOfChunk = sizeOfChunk ? sizeOfChunk : 100000;
			this->allThreads = new std::thread*[nthreads];
			this->duration = 0;
		}


	public:
		template <typename IN, typename OUT, typename ...ARGs>
		void start_init(Scoreboard<IN, OUT> *scoreboard, std::vector<OUT> *output, std::vector<IN> *input, ARGs... args) {
			// create threads
			for (size_t t = 0; t < nthreads; t++) {
				allThreads[t] = new std::thread(&DynamicMapImplementation3<EL>::threadMap<IN, OUT, ARGs...>,this, ((Scoreboard<IN, OUT>*)scoreboard),t, args...);
			}
		}

		template <typename IN, typename OUT, typename ...ARGs>
		void start_analysis(Scoreboard<IN, OUT> * scoreboard, std::vector<OUT> *output, std::vector<IN> *input, ARGs... args) {
			size_t newJobSize = 0;
			while (duration == 0.0f);
			duration = duration * nthreads;

			// analyse worksize
			while (duration > 0.0f && newJobSize < input->size()) {
				tstart = std::chrono::high_resolution_clock::now();
				output->at(newJobSize) = elemental.elemental(input->at(newJobSize), args...);
				tend = std::chrono::high_resolution_clock::now();
				duration -= (double)std::chrono::duration_cast<std::chrono::nanoseconds>(tend - tstart).count();
				newJobSize++;
			}

			// send work size
			while (!scoreboard->lock.try_lock());
			if (newJobSize == input->size()) 
				scoreboard->isFinished = true;
			scoreboard->curIndex = newJobSize;
			scoreboard->jobSize = newJobSize;
			scoreboard->isInitialised = true;
			scoreboard->lock.unlock();
		}


		// Paranthesis operator: call function
		// -----------------------------------
		template<typename IN, typename OUT, typename ...ARGs>
		void operator()(std::vector<OUT> &output, std::vector<IN> &input, ARGs... args) {
			Scoreboard<IN, OUT> * scoreboard = new Scoreboard<IN, OUT>(&input, &output, nthreads, sizeOfChunk);
			
			// start threader
			std::thread *threader;
			tstart = std::chrono::high_resolution_clock::now();
			threader = new std::thread(&DynamicMapImplementation3<EL>::start_init<IN, OUT, ARGs...>, this,scoreboard,  &output, &input, args...);
			tend = std::chrono::high_resolution_clock::now();
			duration = (double)std::chrono::duration_cast<std::chrono::nanoseconds>(tend - tstart).count();

			// main thread analyses
			start_analysis(scoreboard, &output, &input, args...);

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
			delete scoreboard;
		}

		// Friend Functions for Dynamic Map Implementation Class
		// -----------------------------------------------------
		template<typename EL2>
		friend DynamicMapImplementation3<EL2> __DynamicMapWithAccess3(EL2 el, const size_t &threads, const size_t &sizeOfChunk);
	};

	// Friend Functions for Dynamic Map Skeleton Class
	// -----------------------------------------------
	template<typename EL2>
	friend DynamicMapImplementation3<EL2> __DynamicMapWithAccess3(EL2 el, const size_t &threads, const size_t &sizeOfChunk);
};

/*
* We cannot define a friend function with default argument
* that needs access to inner class on latest g++ compiler versions.
* We need a wrapper!
*/
template<typename EL>
DynamicMapSkeleton3::DynamicMapImplementation3<EL> __DynamicMapWithAccess3(EL el, const size_t &threads, const size_t &sizeOfChunk) {
	return DynamicMapSkeleton3::DynamicMapImplementation3<EL>(el, threads, sizeOfChunk);
}

template<typename EL>
DynamicMapSkeleton3::DynamicMapImplementation3<EL> DynamicMap3(EL el, const size_t &threads = 0, const size_t &sizeOfChunk = 0) {
	return __DynamicMapWithAccess3(el, threads, sizeOfChunk);
}


#endif // !SLEDMAP_H
