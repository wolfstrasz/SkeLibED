#ifndef DYNAMICMAP_H
#define DYNAMICMAP_H

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
class DynamicMapSkeleton {

private:
	DynamicMapSkeleton() {}

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
	class DynamicMapImplementation {
	private:
		size_t nthreads;
		size_t sizeOfChunk;
		Elemental<EL> elemental;
		std::thread **allThreads;



		bool isInitialised;
		std::chrono::high_resolution_clock::time_point tstart;
		std::chrono::high_resolution_clock::time_point tend;
		double duration = 0.0f;

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
			// analysis
			//double meanTime;
			size_t startItems;
			void switchWorkload(size_t newMeanWork) {
				this->jobSize = newMeanWork;
				this->jobSize = this->jobSize == 0 ? 1 : jobSize;
			}
			// constructor
			Scoreboard(std::vector<IN> *in, std::vector<OUT> *out, size_t nthreads) {
				this->output = out;
				this->input = in;
				isFinished = false;
				isInitialised = false;
				inputSize = in->size();
				curIndex = 0;
				jobSize = 0;
			}
			~Scoreboard() {}
		};
		void* scoreboard;


		// ThreadMap - function applied to each thread
		// --------------------------------------------
		template<typename IN, typename OUT, typename ...ARGs>
		void threadMap(Scoreboard<IN, OUT> *scoreboard, size_t id, ARGs... args) {
			std::chrono::high_resolution_clock::time_point wstart;
			std::chrono::high_resolution_clock::time_point wend;
			double workTime = 1000000.0f;
			double meanTime = 0.0f;
			size_t meanElements;
			size_t elementsCount = 0;
			size_t elementIndex = 0;

			while (!scoreboard->isInitialised);
			elementsCount = scoreboard->jobSize;

			while (!scoreboard->isFinished) {
				// Lock scoreboard
				while (!scoreboard->scoreboardLock.try_lock()) {
					if (scoreboard->isFinished) break;
				}
				if (scoreboard->isFinished) {
					scoreboard->scoreboardLock.unlock();
					break;
				}

				workTime = workTime / 1000000;
				meanElements = elementsCount / workTime;
				scoreboard->switchWorkload(meanElements);

				// get new data
				if (scoreboard->curIndex + scoreboard->jobSize < scoreboard->inputSize) {
					// set new jobSize

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
				wstart = std::chrono::high_resolution_clock::now();
				for (int elementsFinished = 0; elementsFinished < elementsCount; elementsFinished++) {
					scoreboard->output->at(elementIndex + elementsFinished) = elemental.elemental(scoreboard->input->at(elementIndex + elementsFinished), args...);
				}
				wend = std::chrono::high_resolution_clock::now();
				workTime = (double)std::chrono::duration_cast<std::chrono::nanoseconds>(wend - wstart).count();


			}

		}

		// Constructor
		// -----------
		DynamicMapImplementation(Elemental<EL> elemental, size_t threads, size_t sizeOfChunk) : elemental(elemental) {
			this->nthreads = threads ? threads : std::thread::hardware_concurrency();
			this->sizeOfChunk = sizeOfChunk ? sizeOfChunk : 100000;
			this->duration = 0;
		}


	public:
		template <typename IN, typename OUT, typename ...ARGs>
		void start_init(std::vector<OUT> *output, std::vector<IN> *input, ARGs... args) {

			// create threads
			for (size_t t = 0; t < nthreads; t++) {
				allThreads[t] = new std::thread(&DynamicMapImplementation<EL>::threadMap<IN, OUT, ARGs...>, this, ((Scoreboard<IN, OUT>*)scoreboard), t, args...);
			}
		}

		template <typename IN, typename OUT, typename ...ARGs>
		void start_analysis(std::vector<OUT> *output, std::vector<IN> *input, ARGs... args) {
			double workTime = 1000000.0f;
			double meanTime = 0.0f;
			size_t meanElements = 0;
			size_t elementsCount = 0;
			size_t elementIndex = 0;

			// Scoreboard access analysis
			// -------------------------------------------------------------------------------
			tstart = std::chrono::high_resolution_clock::now();
			while (!((Scoreboard<IN, OUT>*)scoreboard)->scoreboardLock.try_lock());
			if (((Scoreboard<IN, OUT>*)scoreboard)->isFinished) {
				((Scoreboard<IN, OUT>*)scoreboard)->scoreboardLock.unlock();
			}
			workTime = workTime / 1000000;
			meanElements = elementsCount / workTime;
			((Scoreboard<IN, OUT>*)scoreboard)->switchWorkload(meanElements);

			elementsCount = ((Scoreboard<IN, OUT>*)scoreboard)->jobSize;
			elementIndex = ((Scoreboard<IN, OUT>*)scoreboard)->curIndex;
			((Scoreboard<IN, OUT>*)scoreboard)->curIndex += ((Scoreboard<IN, OUT>*)scoreboard)->jobSize;

			((Scoreboard<IN, OUT>*)scoreboard)->scoreboardLock.unlock();
			tend = std::chrono::high_resolution_clock::now();
			// -------------------------------------------------------------------------------




			size_t newJobSize = 0;
			duration = (double)std::chrono::duration_cast<std::chrono::nanoseconds>(tend - tstart).count() * nthreads; // overall communication time
	
				tstart = std::chrono::high_resolution_clock::now();
				for (int i =0; i < nthreads; i++){
					output->at(newJobSize) = elemental.elemental(input->at(newJobSize), args...);
				}
				tend = std::chrono::high_resolution_clock::now();
				meanTime = (double)std::chrono::duration_cast<std::chrono::nanoseconds>(tend - tstart).count() / nthreads; // mean time per item
				newJobSize = duration/meanTime + 1;
				((Scoreboard<IN, OUT>*)scoreboard)->curIndex = nthreads;

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
		
			// start threader
			std::thread *threader;
			threader = new std::thread(&DynamicMapImplementation<EL>::start_init<IN, OUT, ARGs...>, this, &output, &input, args...);

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
		friend DynamicMapImplementation<EL2> __DynamicMapWithAccess(EL2 el, const size_t &threads, const size_t &sizeOfChunk);
	};

	// Friend Functions for Dynamic Map Skeleton Class
	// -----------------------------------------------
	template<typename EL2>
	friend DynamicMapImplementation<EL2> __DynamicMapWithAccess(EL2 el, const size_t &threads, const size_t &sizeOfChunk);
};

/*
* We cannot define a friend function with default argument
* that needs access to inner class on latest g++ compiler versions.
* We need a wrapper!
*/
template<typename EL>
DynamicMapSkeleton::DynamicMapImplementation<EL> __DynamicMapWithAccess(EL el, const size_t &threads, const size_t &sizeOfChunk) {
	return DynamicMapSkeleton::DynamicMapImplementation<EL>(el, threads, sizeOfChunk);
}

template<typename EL>
DynamicMapSkeleton::DynamicMapImplementation<EL> DynamicMap(EL el, const size_t &threads = 0, const size_t &sizeOfChunk = 0) {
	return __DynamicMapWithAccess(el, threads, sizeOfChunk);
}

#endif // !SLEDMAP_H