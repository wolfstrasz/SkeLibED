#include "Mandelbrot.hpp"

#include <chrono>
#include <iostream>
#include <fstream>
#include <string>

namespace mandelbrot {

    bool pixel_t::operator !=(const pixel_t& a) const
		{
			return (r != a.r || g != a.g || b != a.b);
		}

    pixel_t mandelbrot_elemental(size_t taskid, double magnification, size_t xres, size_t yres, size_t itermax) {
		pixel_t pixel;
		double x, xx, y, cx, cy;
		int iteration, hx, hy, i;
		hx = taskid % xres;
		hy = taskid / yres;
		cx = (((float)hx) / ((float)xres) - 0.5) / magnification * 3.0 - 0.7;
		cy = (((float)hy) / ((float)yres) - 0.5) / magnification * 3.0;
		x = 0.0; y = 0.0;
		int colortint = 0;

		for (iteration = 1; iteration < itermax; iteration++) {
			xx = x * x - y * y + cx;
			y = 2.0*x*y + cy;
			x = xx;
			if (x*x + y * y > 10000.0) { colortint = iteration; iteration = itermax + 1; }
		}

		if (iteration <= itermax) {
			pixel.r = 0; pixel.g = 150; pixel.b = 150;
		}
		else {
			pixel.r = 255; pixel.g = 0; pixel.b = 0;
		}

		return pixel;
	}


    void test(size_t threadcount, size_t blockcount, size_t ixc, size_t iyc, size_t itermax, double arg) {
		int itemcount = ixc * iyc;

		//initialisation
		std::vector <int> in(itemcount);
		std::vector <pixel_t> dynMapOut(itemcount);
		std::vector <pixel_t> mapOut(itemcount);

		for (size_t i = 0; i < itemcount; i++) {
			in[i] = i;
		}

		std::chrono::duration<double, std::milli> time;

		// Static Map
		// ----------------------------------------------------------
		auto start = std::chrono::system_clock::now();

		auto map = Map(mandelbrot_elemental, threadcount, blockcount);
		map(mapOut, in, arg, ixc, iyc, itermax);

		auto end = std::chrono::system_clock::now();
		time = end - start;
		std::cout << "SMAP: " << std::to_string(time.count()) << std::endl;

		// Dynamic map
		// ----------------------------------------------------------
		start = std::chrono::system_clock::now();

		auto dynamicMap = DynamicMap(mandelbrot_elemental, threadcount);
		dynamicMap(dynMapOut, in, arg, ixc, iyc, itermax);

		end = std::chrono::system_clock::now();
		time = end - start;

		std::cout << "DMAP: " << std::to_string(time.count()) << std::endl;
	}
}