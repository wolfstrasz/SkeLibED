#pragma once
#ifndef _TEST_MANDELBROT_HPP
#define _TEST_MANDELBROT_HPP

#include "Map.hpp"
#include "DynamicMap1.hpp"
#include "DynamicMap2.hpp"
#include "DynamicMap5.hpp"

#include <chrono>
#include <iostream>
#include <fstream>
#include <string>



namespace mandelbrot {
#define mandelbrot_testcount 1

	struct pixel_t {
		int r;
		int g;
		int b;

		// Operator overtide for !=
		bool operator !=(const pixel_t& a) const
		{
			return (r != a.r || g != a.g || b != a.b);
		}
	};

	pixel_t mandelbrot_elemental(size_t taskid, double magnification, size_t xres, size_t yres, size_t itermax/*, size_t blocksize = 0*/) {

		pixel_t pixel;
		double x, xx, y, cx, cy;
		int iteration, hx, hy, i;
		double magnify = magnification;
		hx = taskid % xres;
		hy = taskid / yres;
		cx = (((float)hx) / ((float)xres) - 0.5) / magnify * 3.0 - 0.7;
		cy = (((float)hy) / ((float)yres) - 0.5) / magnify * 3.0;
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
		//	pixel.g = ((((double)colortint) / itermax) * 255) ;// (colortint % (255/32) )* 32;
		//	if (taskid % (blocksize*2) < blocksize) pixel.b = 0;
		//	else pixel.b = 255;

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
		auto start = std::chrono::system_clock::now();
		time = start - start;


		double max_time = 0;


		//// Static Map
		//// ----------------------------------------------------------
		//for (size_t test = 0; test < mandelbrot_testcount; test++) {
		//	std::cout << "STATIC MAP Test: " << test << std::endl;

		//	auto start = std::chrono::system_clock::now();
		//	auto map = Map(mandelbrot_elemental, threadcount, blockcount);
		//	map(mapOut, in, arg, ixc, iyc, itermax/*, itemcount / (blockcount * threadcount)*/);

		//	auto end = std::chrono::system_clock::now();
		//	time += (end - start);
		//}
		////outfile << "SMAP: " << std::to_string(time.count() / mandelbrot_testcount) << std::endl;
		//std::cout << "SMAP: " << std::to_string(time.count() / mandelbrot_testcount) << std::endl;

		// reset time
		time = start - start;

		// Dynamic map1
		// ----------------------------------------------------------
		for (size_t test = 0; test < mandelbrot_testcount; test++) {
			std::cout << "DYNAMIC MAP1 Test: " << test << std::endl;

			auto start = std::chrono::system_clock::now();

			auto dynamicMap = DynamicMap1(mandelbrot_elemental, threadcount, (itemcount / (blockcount * threadcount)));
			dynamicMap(dynMapOut, in, arg, ixc, iyc, itermax /*, itemcount / (blockcount * threadcount)*/);

			auto end = std::chrono::system_clock::now();
			time += (end - start);

		}
		std::cout << "DMAP1: " << std::to_string(time.count() / mandelbrot_testcount) << std::endl;

		time = start - start;

		// Dynamic map2
	   // ----------------------------------------------------------
		for (size_t test = 0; test < mandelbrot_testcount; test++) {
			std::cout << "DYNAMIC MAP2 Test: " << test << std::endl;

			auto start = std::chrono::system_clock::now();

			auto dynamicMap = DynamicMap2(mandelbrot_elemental, threadcount, (itemcount / (blockcount * threadcount)));
			dynamicMap(dynMapOut, in, arg, ixc, iyc, itermax /*, itemcount / (blockcount * threadcount)*/);

			auto end = std::chrono::system_clock::now();
			time += (end - start);

		}
		std::cout << "DMAP2: " << std::to_string(time.count() / mandelbrot_testcount) << std::endl;

		time = start - start;


		//time = start - start;
		//dynMapOut = std::vector<pixel_t>(itemcount);

		//// Dynamic map 5 
  // // ----------------------------------------------------------
		//for (size_t test = 0; test < mandelbrot_testcount; test++) {
		//	std::cout << "DYNAMIC MAP5 Test: " << test << std::endl;

		//	auto start = std::chrono::system_clock::now();

		//	auto dynamicMap = DynamicMap5(mandelbrot_elemental, threadcount, (itemcount / (blockcount * threadcount)));
		//	dynamicMap(dynMapOut, in, arg, ixc, iyc, itermax /*, itemcount / (blockcount * threadcount)*/);

		//	auto end = std::chrono::system_clock::now();
		//	time += (end - start);
		//	std::cout << " TEST: " << std::to_string((end - start).count()) << "\n";
		//	// Check if output is same
	 //  // ----------------------------------------------------------
		//	bool same = true;
		//	int counter = 0;
		//	for (size_t i = 0; i < itemcount; i++) {
		//		if (dynMapOut[i] != mapOut[i]) {
		//			counter++;
		//			same = false;
		//		}
		//	}
		//	if (!same) {
		//		std::cout << "ERROR:" << counter << "\n";
		//	}
		//	dynMapOut = std::vector<pixel_t>(itemcount);

		//}
		//std::cout << "DMAP5: " << std::to_string(time.count() / mandelbrot_testcount) << std::endl;


		//time = start - start;


		//// Check if output is same
		//// ----------------------------------------------------------
		//bool same = true;
		//for (size_t i = 0; i < itemcount; i++) {
		//	if (dynMapOut[i] != mapOut[i]) {
		//		same = false;
		//		break;
		//	}
		//}
		//if (!same) {
		//	for (size_t i = 0; i < itemcount; i += 10000) {
		//		if (dynMapOut[i] != mapOut[i])std::cout << i << std::endl;
		//	}
		//}
	}
}
#endif
