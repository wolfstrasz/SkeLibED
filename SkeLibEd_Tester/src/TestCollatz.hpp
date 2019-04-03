#ifndef _TEST_COLLATZ_HPP
#define _TEST_COLLATZ_HPP

#include "Map.hpp"
//#include "DynamicMap1.hpp"
//#include "DynamicMap2.hpp"
#include "DynamicMap5.hpp"

#include <chrono>
#include <iostream>
#include <fstream>
#include <string>

namespace collatz {
#define collatz_test_count 5

	void collatz_wait(size_t n = 871) {
		if (n < 1) return;
		while (n != 1) {
			if (n % 2) n = 3 * n + 1;
			else n = n / 2;
		}
	}

	int collatz_elemental(size_t taskid, size_t b) {
		collatz_wait(taskid);
		return taskid + b;
	}

	void test(size_t threadcount, size_t blockcount, size_t itemcount, double arg) {

		// initialisation
		std::vector<int> in(itemcount);
		std::vector<int> dynMapOut(itemcount);
		std::vector<int> mapOut(itemcount);

		for (size_t i = 0; i < itemcount; i++) {
			in[i] = i;
		}
		std::chrono::duration<double, std::milli> time;
		auto start = std::chrono::system_clock::now();
		time = start - start;

		double max_time = 0;

		//// Static Map
		//// ----------------------------------------------------------
		//std::cout << "RUNNING FOR BLOCKS: " << blockcount << "\n --------------------------------------------------\n";

		for (size_t test = 0; test < collatz_test_count; test++) {
			//std::cout << "STATIC MAP Test: " << test << std::endl;
			auto start = std::chrono::system_clock::now();

			auto map = Map(collatz_elemental, threadcount);
			map(mapOut, in, arg);
			auto end = std::chrono::system_clock::now();

			time += (end - start);
			max_time = max_time > (end - start).count() ? max_time : (end - start).count();
		}
		std::cout << "SMAP: " << std::to_string(time.count() / collatz_test_count) << std::endl;
		//std::cout << "--error: " << (max_time / 1000000)  - (time.count() / collatz_test_count) << "\n\n";
		time = start - start;

		//max_time = 0;
		//for (size_t bs = 4; bs < 4096; bs *= 2) {
		//	std::cout << "RUNNING FOR BLOCKS: " << bs << "\n --------------------------------------------------\n";
		//	// Static Map
		//	// ----------------------------------------------------------
		//	for (size_t test = 0; test < collatz_test_count; test++) {
		//		//std::cout << "STATIC MAP Test: " << test << std::endl;
		//		auto start = std::chrono::system_clock::now();

		//		auto map = Map(collatz_elemental, threadcount, bs);
		//		map(mapOut, in, arg);
		//		auto end = std::chrono::system_clock::now();

		//		time += (end - start);
		//		max_time = max_time > (end - start).count() ? max_time : (end - start).count();
		//	}
		//	std::cout << "SMAP: " << std::to_string(time.count() / collatz_test_count) << std::endl;
		//	std::cout << "--error: " << (max_time / 1000000) - (time.count() / collatz_test_count) << "\n\n";
		//	// reset time
		//	time = start - start;
		//	max_time = 0;
		//}

		// Dynamic Map
		// ----------------------------------------------------------
		for (size_t test = 0; test < collatz_test_count; test++) {
			std::cout << "DYNAMIC MAP5 Test: " << test << std::endl;
			auto start = std::chrono::system_clock::now();

			auto dynamicMap = DynamicMap5(collatz_elemental, threadcount, itemcount / (blockcount * threadcount));
			//	auto dynamicMap = DynamicMap(collatz_elemental);
			//	std::cout << "DYNAMIC MAP Test: " << test << std::endl;
			dynamicMap(dynMapOut, in, arg);
			//dynamicMap.stop();

			auto end = std::chrono::system_clock::now();
			time += (end - start);
		}
		std::cout << "DMAP5: " << std::to_string(time.count() / collatz_test_count) << std::endl;

		// Check if output is same
		// ----------------------------------------------------------
		bool same = true;
		for (size_t i = 0; i < itemcount; i++) {
			if (dynMapOut[i] != mapOut[i]) {
				same = false;
				break;
			}
		}
		if (same)std::cout << "SAME OUTPUT" << std::endl;
		std::cout << dynMapOut[1234] << std::endl;
		if (!same) {
			for (size_t i = 0; i < itemcount; i += 10000) {
				if (dynMapOut[i] != mapOut[i])std::cout << i << std::endl;
			}
		}

	}

}


#endif
