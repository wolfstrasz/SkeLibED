#include "Collatz.hpp"

#include <chrono>
#include <iostream>
#include <fstream>
#include <string>

namespace collatz {

    void collatz_wait(size_t n = 871){
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

		// Initialisation of test
        // ----------------------------------------------------------
		std::vector<int> in(itemcount);
		std::vector<int> dynMapOut(itemcount);
		std::vector<int> mapOut(itemcount);

		for (size_t i = 0; i < itemcount; i++) {
			in[i] = i;
		}

		std::chrono::duration<double, std::milli> time;

		// Static Map
		// ----------------------------------------------------------
		auto start = std::chrono::system_clock::now();

		auto map = Map(collatz_elemental, threadcount, blockcount);
		map(mapOut, in, arg);

		auto end = std::chrono::system_clock::now();
		time = end - start;
		std::cout << "SMAP: " << std::to_string(time.count()) << std::endl;

		// Dynamic Map
		// ----------------------------------------------------------
		start = std::chrono::system_clock::now();

		auto dynamicMap = DynamicMap(collatz_elemental, threadcount, itemcount / (blockcount * threadcount));
		dynamicMap(dynMapOut, in, arg);

		end = std::chrono::system_clock::now();
		time = end - start;
		std::cout << "DMAP: " << std::to_string(time.count()) << std::endl;

		// Analysis for debugging
		// ----------------------------------------------------------
		bool same = true;
		for (size_t i = 0; i < itemcount; i++) {
			if (dynMapOut[i] != mapOut[i]) {
				same = false;
				break;
			}
		}
		if (same) {
            std::cout << "SAME OUTPUT" << std::endl;
            return;
        }
	
        for (size_t i = 0; i < itemcount; i += 10000) {
            if (dynMapOut[i] != mapOut[i])std::cout << i << std::endl;
        }
	}
}