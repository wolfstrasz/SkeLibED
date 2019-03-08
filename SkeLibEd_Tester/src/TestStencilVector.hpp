#ifndef _TEST_STENCIL_VECTOR_HPP
#define _TEST_STENCIL_VECTOR_HPP
#include <chrono>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <utility>
#include "Stencil.hpp"
#include "DynamicMap6.hpp"
#include "Pixel.hpp"
#include "TestPixel.hpp"

#define SV_DIM 10

Pattern os;
std::vector<float> in;
std::vector<float> sout;
std::vector<float> pout;

void testStencilVector() {
	std::cout << "here\n";
	std::cout << &in<< std::endl;
	// Initialise offsets
	os.add(0, 0);
	os.add(-1, 0);
	os.add(0, 1);
	os.add(1, 0);
	os.add(0, -1);

	//in = std::vector<float>(SV_DIM * SV_DIM);

	// initialise input
	for (int i=0; i < SV_DIM; i++) {
		for (int j = 0; j < SV_DIM; j++) {
			in.push_back(i*SV_DIM + j);
			std::cout << in.at(i*SV_DIM + j)<<" ";
		}
		std::cout << std::endl;
	}


	pout = std::vector<float>(in.size());
	sout = std::vector<float>(in.size());
	std::cout << "here\n";

	auto stencil = Stencil(1);
	stencil(pout, in, os, PSLED_BORDER, SV_DIM, SV_DIM);

	for (int i = 0; i < SV_DIM; i++) {
		for (int j = 0; j < SV_DIM; j++) {
			//in.push_back(i*SV_DIM + j);
			std::cout << pout.at(i*SV_DIM + j) << " ";
		}
		std::cout << std::endl;
	}
}

#endif // !_TEST_STENCIL_VECTOR_HPP
