#ifndef TEST_PIXEL_HPP
#define TEST_PIXEL_HPP

#include "Pixel.hpp"
void testPixel() {

	// Initialise pixels
	psled::pPixel a = psled::pPixel(10, 10, 10);
	psled::pPixel b;
	b.r = 11;
	b.g = 22;
	b.b = 33;

	// Test unary addition
	a += b;
	a.printPixel();
	// Test binary addition
	a = a + b;
	a.printPixel();

	// Test unary multiplication	
	a *= 3;
	a.printPixel();

	// Test binary multiplication
	a = a * 3;
	a.printPixel();

	// Test unary division
	a /= 2;
	a.printPixel();

	// Test binary division
	a = a / 4;
	a.printPixel();

	// Test unary subtraction
	a -= b;
	a.printPixel();

	// Test binary subtraction
	a = a - b;
	a.printPixel();

}



#endif // !TEST_PIXEL_HPP
