#ifndef _TEST_IMG_HPP
#define _TEST_IMG_HPP

#include "Pixel.hpp"
#include <string.h>

#define HYRES 100
#define HXRES 100
#define FORMAT 255

void testImgWrite() {
	FILE *outfile;
	FILE *infile;
	std::cout << " TEST IMG WRITE\n";
	outfile = fopen("custompicture1000.ppm", "w");

	psled::Pixel img[HXRES][HYRES];

	/*
		_________
		| B | R |
		| G | W |
		_________

	*/
	std::cout << "new img" << std::endl;
	for (int i = 0; i < HXRES; i++){
		for (int j = 0; j < HYRES; j++) {
			if (i < HXRES / 2 && j < HYRES / 2) {
				img[i][j].b = 255;
			}
			else if (i < HXRES / 2) {
				img[i][j].r = 255;
			}
			else if (!(i < HXRES / 2) && !(j < HYRES / 2)) {
				img[i][j].g = 255; img[i][j].b = 255; img[i][j].r = 255;
			}
			else {
				img[i][j].g = 255;
			}
		}
	}
	std::cout << " opening file\n";
	fprintf(outfile, "P6\n");
	fprintf(outfile, "%d %d\n%d\n", HYRES, HXRES, FORMAT);
	for (int i = 0; i < HXRES; i++) {
		for (int j = 0; j < HYRES; j++) {
			fputc((char)img[i][j].r, outfile);
			fputc((char)img[i][j].g, outfile);
			fputc((char)img[i][j].b, outfile);
		}
	}
	fclose(outfile);
	std::cout << " file closed\n";
}

void testImgRead() {
	int height, width, format;
	psled::Pixel img[300][300]; /* cygwin testing is bad as it cannot get enough memory*/

	char six[100];
	char *pSix = six;
	std::cout << " TEST IMG READ \n";
	infile = fopen("scratches.ppm", "r");
	outfile = fopen("scratches2.ppm", "w");

	std::cout << "\nreading pSix: \n";
	fscanf(infile, "%s\n", pSix);
	printf(pSix);

	//while (six[0] != '\n') {
	//	fscanf(infile, "%c", pSix);
	//	printf("%c", pSix[0]);
	//}

	fscanf(infile, "%d\n %d\n%d\n", &height, &width, &format);
	std::cout << "(" << height << ", " << width << ", " << format << ")\n";


	//
	//for (size_t i = 0; i < *height; i++) {
	//	for (int j = 0; j < *weight; j++) {
	//		fputc((char)img[i][j].r, outfile);
	//		fputc((char)img[i][j].g, outfile);
	//		fputc((char)img[i][j].b, outfile);
	//	}
	//}

	std::cout << "reading colors\n";
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			char c;
			fscanf(infile, "%c", &c);
			img[i][j].r = (int)c;
			fscanf(infile, "%c", &c);
			img[i][j].g = (int)c;
			fscanf(infile, "%c", &c);
			img[i][j].b = (int)c;
		}
	}

	std::cout << "writing to img\n";
	fprintf(outfile, "P6\n");
	fprintf(outfile, "%d %d\n%d\n", height, width, format);
	for (size_t i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			fputc((char)img[i][j].r, outfile);
			fputc((char)img[i][j].g, outfile);
			fputc((char)img[i][j].b, outfile);
		}
	}
	std::cout << "closing files\n";
	fclose(outfile);
	fclose(infile);
}

#endif // !_TEST_IMG_HPP
