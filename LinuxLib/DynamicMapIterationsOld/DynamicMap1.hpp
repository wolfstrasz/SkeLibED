#ifndef DYNAMICMAP1_HPP
#define DYNAMICMAP1_HPP

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

class DynamicMapSkeleton1 {

private:
	DynamicMapSkeleton1() {}

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
	class DynamicMapImplementation1 {

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

			/* SPECIALIZED */
			std::vector<bool> isThereWork;
			std::vector<size_t> elementsCount;
			std::vector<size_t> threadInputIndex;
			std::queue <size_t> finishedWorkers;
			/////////////////

			Scoreboard(std::vector<IN> *in, std::vector<OUT> *out, size_t nthreads, size_t sizeOfChunk) : output(out), input(in), jobSize(sizeOfChunk) {
				/* SPECIALIZED */
				isThereWork = std::vector<bool>(nthreads);
				elementsCount = std::vector <size_t>(nthreads);
				threadInputIndex = std::vector <size_t>(nthreads);
				for (size_t t = 0; t < nthreads; t++) {
					finishedWorkers.push(t);
				}
				/////////////////
			}

			~Scoreboard() {}
		};

		// ThreadMap - function applied to each thread
		// --------------------------------------------
		template<typename IN, typename OUT, typename ...ARGs>
		void threadMap(Scoreboard<IN, OUT> *scoreboard, size_t threadID, ARGs... args) {

			auto input = scoreboard->input;
			auto output = scoreboard->output;

			while (!scoreboard->isFinished) {
				if (scoreboard->isThereWork[threadID]) {

					size_t elementsCount = scoreboard->elementsCount[threadID];
					size_t elementIndex = scoreboard->threadInputIndex[threadID];

					// Process the data block
					// ----------------------
					for (int elementsFinished = 0; elementsFinished < elementsCount; elementsFinished++) {
						output->at(elementIndex+elementsFinished) = elemental.elemental(input->at(elementIndex+elementsFinished), args...);
					}

					scoreboard->isThereWork[threadID] = false;
					while (!scoreboard->lock.try_lock());
					scoreboard->finishedWorkers.push(threadID);
					scoreboard->lock.unlock();
				}
			}

		}

		// Constructor
		// -----------
		DynamicMapImplementation1(Elemental<EL> elemental, size_t threads, size_t sizeOfChunk) : elemental(elemental) {
			this->nthreads = threads ? threads : std::thread::hardware_concurrency();
			this->sizeOfChunk = sizeOfChunk ? sizeOfChunk : 100000;
		}



	public:
		// Paranthesis operator: call function
		// -----------------------------------
		template<typename IN, typename OUT, typename ...ARGs>
		void operator()(std::vector<OUT> &output, std::vector<IN> &input, ARGs... args) {
			this->allThreads = new std::thread*[nthreads];
			Scoreboard<IN, OUT> *scoreboard  = new Scoreboard<IN, OUT>(&input, &output, nthreads, this->sizeOfChunk);

			// Run threads
			// -----------
			for (size_t t = 0; t< nthreads; t++) allThreads[t] = new std::thread(&DynamicMapImplementation1<EL>::threadMap<IN, OUT, ARGs...>, this, scoreboard, t, args...);

			
			// Assign new jobs until work is done
			// ----------------------------------
			while (!scoreboard->isFinished) {

				// Check for free thread
				if (!scoreboard->finishedWorkers.empty()) {
					size_t threadID = scoreboard->finishedWorkers.front();
					scoreboard->finishedWorkers.pop();
					if (scoreboard->curIndex +scoreboard->jobSize < scoreboard->input->size()) {
						scoreboard->elementsCount[threadID] = scoreboard->jobSize;
						scoreboard->threadInputIndex[threadID] = scoreboard->curIndex;
						scoreboard->curIndex += scoreboard->jobSize;
						scoreboard->isThereWork[threadID] = true;
					} else {
						size_t lastItemsCount = scoreboard->input->size() - scoreboard->curIndex;
						scoreboard->elementsCount[threadID] = lastItemsCount;
						scoreboard->threadInputIndex[threadID] = scoreboard->curIndex;
						scoreboard->curIndex += lastItemsCount;
						scoreboard->isThereWork[threadID] = true;
					}
				}

				// Check if job will be finished
				if (scoreboard->curIndex == scoreboard->input->size())
					scoreboard->isFinished = true;

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
		friend DynamicMapImplementation1<EL2> __DynamicMapWithAccess1(EL2 el, const size_t &threads, const size_t &sizeOfChunk);
	};

	// Friend Functions for Dynamic Map Skeleton Class
	// -----------------------------------------------
	template<typename EL2>
	friend DynamicMapImplementation1<EL2> __DynamicMapWithAccess1(EL2 el, const size_t &threads, const size_t &sizeOfChunk);
};

/*
* We cannot define a friend function with default argument
* that needs access to inner class on latest g++ compiler versions.
* We need a wrapper!
*/
template<typename EL>
DynamicMapSkeleton1::DynamicMapImplementation1<EL> __DynamicMapWithAccess1(EL el, const size_t &threads, const size_t &sizeOfChunk) {
	return DynamicMapSkeleton1::DynamicMapImplementation1<EL>(el, threads, sizeOfChunk);
}

template<typename EL>
DynamicMapSkeleton1::DynamicMapImplementation1<EL> DynamicMap1(EL el, const size_t &threads = 0, const size_t &sizeOfChunk = 0) {
	return __DynamicMapWithAccess1(el, threads, sizeOfChunk);
}

#endif // !SLEDMAP_H
