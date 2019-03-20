#ifndef _PIXEL_HPP
#define _PIXEL_HPP
#include <cmath>
namespace psled {

	struct Pixel {
		int r, g, b;
		Pixel(int r = 0, int g = 0, int b = 0) { this->r = r; this->g = g; this->b = b; }
		bool non = true;

		// Overload unary operators
		// ------------------------
		struct Pixel& operator+= (const Pixel& rhs) { r += rhs.r;  g += rhs.g; b += rhs.b; return *this; }
		struct Pixel& operator*= (const int &k) { r *= k; g *= k; b *= k; return *this; }
		struct Pixel& operator/= (const int &k) {
			r /= k;
			g /= k;
			b /= k;

			// // Mirror if lower than 0
			if (r < 0) r = -r;
			if (g < 0) g = -g;
			if (b < 0) b = -b;
			//
			// // Mirror if higher than 255
			if (r > 255) r = 255 - (r % 256);
			if (g > 255) g = 255 - (g % 256);
			if (b > 255) b = 255 - (b % 256);

			// r = (r+256) % 256;
			// g = (g+256) % 256;
			// b = (b+256) % 256;
			 return *this;
		}
		struct Pixel& operator-= (const Pixel& rhs) { r -= rhs.r; g -= rhs.g; b -= rhs.b; return *this; }

	};

	// Overload binary operators
	// -------------------------
	Pixel operator+(Pixel lhs, const Pixel& rhs) { return lhs += rhs; }
	Pixel operator-(Pixel lhs, const Pixel& rhs) { return lhs -= rhs; }
	Pixel operator*(Pixel lhs, const int k) { return lhs *= k; }
	Pixel operator*(const int k, Pixel rhs) { return rhs *= k; }
	Pixel operator/(Pixel lhs, const int k) { return lhs /= k; }

	// Overload print operator
	std::ostream & operator << (std::ostream &out, const Pixel &p)
	{
		out << "(R, G, B | NON) = (" << p.r << ", " << p.g << ", " << p.b << "| " << (int)p.non << ") ";
		return out;
	}

}

/* Depricated funcionality for now
		// void operator=(const Pixel &p) {
		// //	std::cout << "  (NONI  => " << int(non) << ")  ";
		// 	non = false;
		// 	r = p.r;
		// 	g = p.g;
		// 	b = p.b;
		// }
		// void operator= (const int &a) {
		// //	std::cout << "  (NONI  => " << int(non) << ")  + CONST INT =  " << a << "   ";
		// 	r = a;
		// 	g = a;
		// 	b = a;
		// 	if (a == 0)
		// 		non = true;
		// }
		// void printPixel() {
		// 	std::cout << "(R, G, B | NON) = (" << r << ", " << g << ", " << b << "| "<< (int)non <<  ")\n";
		// }


			struct Pixel& operator+= (const Pixel& rhs) {
		//	std::cout << "  (NONI  = " << int(non) << ")  ";
			// if (non) {
			// 	r = rhs.r;
			// 	g = rhs.g;
			// 	b = rhs.b;
			// }
			// else {
			// 	int col1 = r * r;
			// 	int col2 = rhs.r * rhs.r;
			// 	int avgCol = (col1 + col2) / 2;
			// 	r = sqrt(avgCol);
			// 	//r += rhs.r;
			// 	col1 = g * g;
			// 	col2 = rhs.g * rhs.g;
			// 	avgCol = (col1 + col2) / 2;
			// 	g = sqrt(avgCol);
			// 	//g += rhs.g;
			// 	col1 = b * b;
			// 	col2 = rhs.b * rhs.b;
			// 	avgCol = (col1 + col2) / 2;
			// 	b = sqrt(avgCol);
			// 	//b += rhs.b;
			// }
			//std::cout << "Adding: ";
			//std::cout << "(R, G, B | NON) = (" << rhs.r << ", " << rhs.g << ", " << rhs.b << "| "<< (int)non <<  ")\n";
			//std::cout << "To :";
			//	this->printPixel();
			 r += rhs.r;
			 g += rhs.g;
			 b += rhs.b;
			return *this;
		}

		struct Pixel& operator/= (const int &k) {
		//	if (r < 0 ) std::cout << "Old R: " << r << " New R:";
			r /= k;
			if (r < 0) r = -r;
			if (r > 255) r = 255 - (r % 255);
			//r +=256;
			//r %=256;
	//		std::cout << r << "\n";
			g /= k;
			if (g < 0) g = -g;
			if (g > 255) g = 255 - (g % 255);
		//	g +=256;
		//	g %=256;
			b /= k;
			if (b < 0) b = -b;
			if (b > 255) b = 255 - (b % 255);
		//	b +=256;
		//	b %=256;
			 return *this; }
*/
#endif // !_PIXEL_HPP
