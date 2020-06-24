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

#include "Scoreboard.hpp"

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
		Elemental<EL> elemental;
		std::vector<std::thread *>allThreads;

		std::chrono::high_resolution_clock::time_point tstart;
		std::chrono::high_resolution_clock::time_point tend;

		// Constructor
		// -----------
		DynamicMapImplementation(Elemental<EL> elemental, size_t threads) : elemental(elemental) {
			this->nthreads = threads ? threads : std::thread::hardware_concurrency();
			allThreads.reserve(nthreads);
		}

		// Function applied to each thread
		// -------------------------------
		template<typename IN, typename OUT, typename ...ARGs>
		void threadMap(Scoreboard<IN, OUT> *scoreboard, size_t id, ARGs... args) {
			// Local timing data
			std::chrono::high_resolution_clock::time_point wstart;
			std::chrono::high_resolution_clock::time_point wend;

			// Local task execution data
			double workTime = 1000000.0f;
			size_t taskItemsCount = 0;
			size_t taskStartItemIndex = 0;

			// Read-only barrier to stop threads from executing tasks untill main thread provides the initial analysis
			while (!scoreboard->isInitialised);
			taskItemsCount = scoreboard->jobSize;

			// Work until there is no more work
			while (!scoreboard->isFinished) {

				// Lock scoreboard
				while (!scoreboard->scoreboardLock.try_lock()) {
					if (scoreboard->isFinished) break;
				}
				if (scoreboard->isFinished) {
					scoreboard->scoreboardLock.unlock();
					break;
				}
				// Accesses the scoreboard 
				scoreboard->GetNewTask(workTime, taskItemsCount, taskStartItemIndex);

				// Unlock scoreboard
				scoreboard->scoreboardLock.unlock();

				// Process the data block and measure the execution time
				wstart = std::chrono::high_resolution_clock::now();
				for (int elementsFinished = 0; elementsFinished < taskItemsCount; elementsFinished++) {
					scoreboard->output->at(taskStartItemIndex + elementsFinished) = 
						elemental.elemental(scoreboard->input->at(taskStartItemIndex + elementsFinished), args...);
				}
				wend = std::chrono::high_resolution_clock::now();
				workTime = (double)std::chrono::duration_cast<std::chrono::nanoseconds>(wend - wstart).count();
			}
		}


		template <typename IN, typename OUT, typename ...ARGs>
		void start_analysis(Scoreboard<IN, OUT> * scoreboard, std::vector<OUT> *output, std::vector<IN> *input, ARGs... args) {
			// Scoreboard access time analysis - calculate time a thread will access the scoreboard
			// -----------------------------------------------------------------------------------
			
			// Data that each thread will use in communication with the scoreboard
			double workTime = 1000000.0f;
			size_t taskItemsCount = 0;
			size_t taskStartItemIndex = 0;

			tstart = std::chrono::high_resolution_clock::now();
			// Lock scoreboard
			while (!scoreboard->scoreboardLock.try_lock());
			if (scoreboard->isFinished) {
				scoreboard->scoreboardLock.unlock();
			}

			// Access scoreboard
			scoreboard->GetNewTask(workTime, taskItemsCount, taskStartItemIndex);

			// Unlock scoreboard
			scoreboard->scoreboardLock.unlock();
			tend = std::chrono::high_resolution_clock::now();

			// Time for work should be as equal as possible to the time each thread accesses the scoreboard in a row
			// T_work = T_thread_communication * Nthreads
			// -----------------------------------------------------------------------------------------------------

			double t_thread_communication = (double)std::chrono::duration_cast<std::chrono::nanoseconds>(tend - tstart).count();
			double t_work =  t_thread_communication* nthreads; // overall communication time
	
			// Calcualte how fast the first Nthread items can be processed (can be optimised with a better value)
			tstart = std::chrono::high_resolution_clock::now();
			for (int index = 0; index < nthreads; index++){
				output->at(index) = elemental.elemental(input->at(index), args...);
			}
			tend = std::chrono::high_resolution_clock::now();

			// Obtain mean time for each element
			double meanTime = (double)std::chrono::duration_cast<std::chrono::nanoseconds>(tend - tstart).count() / nthreads; // mean time per item

			// Calculate new size of jobs so that it matches requirement (1)
			size_t newJobSize = t_work/meanTime + 0.5f;
			
			// Set work info and allow threads to process the data
			scoreboard->curIndex = nthreads; // N thread tasks were finished
			scoreboard->jobSize = newJobSize;
			scoreboard->isInitialised = true;
		}

	public:
		// Paranthesis operator: call function
		// -----------------------------------
		template<typename IN, typename OUT, typename ...ARGs>
		void operator()(std::vector<OUT> &output, std::vector<IN> &input, ARGs... args) {
			auto scoreboard = new Scoreboard<IN, OUT>(&input, &output, nthreads);
		
			// Create threads
			for (size_t t = 0; t < nthreads; t++) {
				allThreads.push_back(new std::thread(&DynamicMapImplementation<EL>::threadMap<IN, OUT, ARGs...>, this, scoreboard, t, args...));
			}

			// Main thread analyses and provides initial task size
			start_analysis(scoreboard, &output, &input, args...);

			// Join threads
			for (size_t t = 0; t < nthreads; ++t) {
				allThreads.at(t)->join();
				delete allThreads.at(t);
			}

			delete (scoreboard);
		}

		// Friend Functions for Dynamic Map Implementation Class
		// -----------------------------------------------------
		template<typename EL2>
		friend DynamicMapImplementation<EL2> __DynamicMapWithAccess(EL2 el, const size_t &threads);
	};

	// Friend Functions for Dynamic Map Skeleton Class
	// -----------------------------------------------
	template<typename EL2>
	friend DynamicMapImplementation<EL2> __DynamicMapWithAccess(EL2 el, const size_t &threads);
};

/*
* We cannot define a friend function with default argument
* that needs access to inner class on latest g++ compiler versions.
*/
template<typename EL>
DynamicMapSkeleton::DynamicMapImplementation<EL> __DynamicMapWithAccess(EL el, const size_t &threads) {
	return DynamicMapSkeleton::DynamicMapImplementation<EL>(el, threads);
}

template<typename EL>
DynamicMapSkeleton::DynamicMapImplementation<EL> DynamicMap(EL el, const size_t &threads = 1) {
	return __DynamicMapWithAccess(el, threads);
}

#endif
