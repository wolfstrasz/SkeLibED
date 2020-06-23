#ifndef _TEST_COLLATZ_HPP
#define _TEST_COLLATZ_HPP

#include "../Skeletons/Map.hpp"
#include "../Skeletons/DynamicMap.hpp"


namespace collatz {

	// Collatz function to simulate some work
	void collatz_wait(size_t n);

	// Elemental to use in the maps
	int collatz_elemental(size_t taskid, size_t b);

	// Executes static and dynamic map tests
	void test(size_t threadcount, size_t blockcount, size_t itemcount, double arg);

}


#endif
