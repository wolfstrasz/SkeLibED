#ifndef _TEST_BLUR_HPP
#define _TEST_BLUR_HPP

#include "Pixel.hpp"
#include <string.h>
#include "Stencil.hpp"
#include <fstream>

namespace test_blur {

#define BLUR_FORMAT 255
#define BLUR_THREADCOUNT 1


	FILE *outfile;
	FILE *grayscaleOutFile;
	FILE *infile;
	int height, width;
	std::vector<psled::Pixel> imgIn;
	std::vector<psled::Pixel> imgOut;
	std::string filenameout;
	std::string filenamein;
	std::string gsfilenameOut;
	Pattern gaussianBlur;


	psled::Pixel grayscalePixel(psled::Pixel pixel){
		float Y = (0.299f * pixel.r) + (0.587 * pixel.g) + (0.114 * pixel.b);
		return  psled::Pixel (Y,Y,Y);
	}
	void test(/*std::string filename, int h, int w*/) {
		//height = h;
		//width = w;
		width = 497;
		height = 373;
		std::string filename = "bunny";
		char six[100];
		char *pSix = six;



		auto stencil = Stencil(BLUR_THREADCOUNT);

		filenameout = filename;
		filenamein = filename;
		gsfilenameOut = filename;

		filenamein.append(".ppm");
		filenameout.append("_blurredtest44.ppm");
		gsfilenameOut.append("_grayscale44.ppm");

		infile = fopen(filenamein.c_str(), "r");
		outfile = fopen(filenameout.c_str(), "w");
		grayscaleOutFile = fopen(gsfilenameOut.c_str(), "w");


		printf(filenamein.c_str());
		printf("\n");
		printf(filenameout.c_str());
		printf("\n");
		printf(gsfilenameOut.c_str());
		printf("\n");

		// read img file
		std::cout << "\nReading pSix: \n";
		fscanf(infile, "%s\n", pSix);
		printf(pSix);
		printf("\n");
		// while (six[0] != '\n') {
		// 	fscanf(infile, "%c", pSix);
		// 	printf("%c", pSix[0]);
		// }
		int he, wi, fo;
		fscanf(infile, "%d\n %d\n%d\n", &wi, &he, &fo);
		std::cout << "(" << wi << ", " << he << ", " << fo << ")\n";
		width = wi;
		height = he;
		imgIn = std::vector<psled::Pixel>(width * height);
		imgOut = std::vector<psled::Pixel>(width * height);
		std::cout << "(" << width << ", " << height << ")\n";
		std::cout << "Reading input colors\n";
		for (int i = 0; i < width; i++) {
			for (int j = 0; j < height; j++) {
				char c;
				 int k;
				fscanf(infile, "%c", &c);
				// if ((int)c < 0)
				// 	k = 128 - (int)c;
				// 	else k = (int)c;
				imgIn.at(i*height + j).r = (c + 256) % 256;
				fscanf(infile, "%c", &c);
				// if ((int)c < 0)
				// 	k = 128 - (int)c;
				// 	else k = (int)c;
				imgIn.at(i*height + j).g = (c + 256) % 256;
				fscanf(infile, "%c", &c);
				// if ((int)c < 0)
				// 	k = 128 - (int)c;
				// 	else k = (int)c;
				imgIn.at(i*height + j).b = (c + 256) % 256;
				//if (imgIn.r == 108)
				//std::cout << "(" << i << ", " <<j <<")";
				//std::cin<<k;
				//REMOVE THIS
				//imgOut.at(i*width + j) = imgIn.at(i*width + j);
			}
		}
		//Grayscale img

		// std::cout << "(" << width << ", " << height << ")\n";
		// std::cout << "writing to img grayscale\n";
		// fprintf(grayscaleOutFile, "P6\n");
		// fprintf(grayscaleOutFile, "%d %d\n%d\n", width, height, BLUR_FORMAT);
		// for (int  i = 0; i < width; i++) {
		// 	for (int j = 0; j < height; j++) {
		// 		psled::Pixel px = grayscalePixel (imgIn.at(i*height + j));
		// 		imgIn.at(i*height + j) = px;
		// 		//psled::Pixel px = (imgIn.at(i*height + j));
		//
		// 		fputc((char)(px.r), grayscaleOutFile);
		// 		fputc((char)(px.g), grayscaleOutFile);
		// 		fputc((char)(px.b), grayscaleOutFile);
		// 	}
		// }
		// std::cout << "(" << width << ", " << height << ")\n";
		//

		// // use stencil
		// gaussianBlur.add(-1, -1, 1);
		// gaussianBlur.add(-1, 0, 2);
		// gaussianBlur.add(-1, 1, 1);
		// gaussianBlur.add(0, -1, 2);
		// gaussianBlur.add(0, 0, 4);
		// gaussianBlur.add(0, 1, 2);
		// gaussianBlur.add(1, -1, 1);
		// gaussianBlur.add(1, 0, 2);
		// gaussianBlur.add(1, 1, 1);

// BOX BLUR
		// gaussianBlur.add(-1, -1, 1);
		// gaussianBlur.add(-1, 0, 1);
		// gaussianBlur.add(-1, 1, 1);
		// gaussianBlur.add(0, -1, 1);
		// gaussianBlur.add(0, 0, 1);
		// gaussianBlur.add(0, 1, 1);
		// gaussianBlur.add(1, -1, 1);
		// gaussianBlur.add(1, 0, 1);
		// gaussianBlur.add(1, 1, 1);

// IDENTITY
		//gaussianBlur.add(0,0,1);

// SHARPEN
		gaussianBlur.add(-1, 0, -1);
		gaussianBlur.add(0, -1, -1);
		gaussianBlur.add(0, 0, 5);
		gaussianBlur.add(0, 1, -1);
		gaussianBlur.add(1, 0, -1);
		gaussianBlur.normalization = false;

// LEFT SOBEL
			// gaussianBlur.add(-1, -1, -1);
			// gaussianBlur.add(-1, 0, -1);
			// gaussianBlur.add(-1, 1, -1);
			// gaussianBlur.add(0, -1, -1);
			// gaussianBlur.add(0, 0, 9);
			// gaussianBlur.add(0, 1, -1);
			// gaussianBlur.add(1, -1, -1);
			// gaussianBlur.add(1, 0, -1);
			// gaussianBlur.add(1, 1, -1);
			// gaussianBlur.normalization = false;

	 	std::cout << "running Blur\n";
	 	stencil(imgOut, imgIn, gaussianBlur, PSLED_CROP, height, width);

	    // write img file

		std::cout << "writing to img\n";
		fprintf(outfile, "P6\n");
		fprintf(outfile, "%d %d\n%d\n", width, height, BLUR_FORMAT);
		for (size_t i = 0; i < width; i++) {
			for (int j = 0; j < height; j++) {
				fputc((char)(imgOut.at(i*height + j).r), outfile);
				fputc((char)(imgOut.at(i*height + j).g), outfile);
				fputc((char)(imgOut.at(i*height + j).b), outfile);
			}
		}
		// close files
		std::cout << "closing files: Grayscale\n";
		fclose(grayscaleOutFile);
		std::cout << "closing files: OUT\n";
		fclose(outfile);
		std::cout << "closing files: IN\n";
		fclose(infile);
		std::cout << "finished";
	}
}

#endif // !_TEST_BLUR_HPP
