#ifndef _PIXEL_HPP
#define _PIXEL_HPP

namespace psled {

	struct pPixel {
		int r, g, b;
		pPixel(int r = 0, int g = 0, int b = 0) { this->r = r; this->g = g; this->b = b; }

		// Overload operators
		struct pPixel& operator+= (const pPixel& rhs) { r += rhs.r; g += rhs.g; b += rhs.b; return *this; }
		struct pPixel& operator*= (const int &k) { r *= k; g *= k; b *= k; return *this; }
		struct pPixel& operator/= (const int &k) { r /= k; g /= k; b /= k; return *this; }
		struct pPixel& operator-= (const pPixel& rhs) { r -= rhs.r; g -= rhs.g; b -= rhs.b; return *this; }

		void printPixel() {
			std::cout << "(R, G, B) = (" << r << ", " << g << ", " << b << ")\n";
		}
	};

	// Overload binary operators
	pPixel operator+(pPixel lhs, const pPixel& rhs) { return lhs += rhs; }
	pPixel operator-(pPixel lhs, const pPixel& rhs) { return lhs -= rhs; }
	pPixel operator*(pPixel lhs, const int k) { return lhs *= k; }
	pPixel operator*(const int k, pPixel rhs) { return rhs *= k; }
	pPixel operator/(pPixel lhs, const int k) { return lhs /= k; }

}

#endif // !_PIXEL_HPP

