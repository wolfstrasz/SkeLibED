#pragma once
#ifndef _TEST_MANDELBROT_HPP
#define _TEST_MANDELBROT_HPP

#include "../Skeletons/Map.hpp"
#include "../Skeletons/DynamicMap.hpp"


namespace mandelbrot {

	// Pixel structure for use as a type for the input items
	struct pixel_t {
		int r;
		int g;
		int b;

		bool operator !=(const pixel_t& a) const;
	};

	// Elemental to use in the maps
	pixel_t mandelbrot_elemental(size_t taskid, double magnification, size_t xres, size_t yres, size_t itermax);

	// Executes static and dynamic map tests
	void test(size_t threadcount, size_t blockcount, size_t ixc, size_t iyc, size_t itermax, double arg);
}
#endif
