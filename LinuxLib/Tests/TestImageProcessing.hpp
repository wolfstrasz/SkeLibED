#ifndef _TEST_IMAGE_PROCESSING_HPP
#define _TEST_IMAGE_PROCESSING_HPP

#include "Pixel.hpp"
#include <string.h>
#include "../Skeletons/Stencil.hpp"
#include <fstream>

namespace test_img_pr {

#define IMG_PR_FORMAT 255
#define IMG_PR_THREADCOUNT 4


	FILE *outfile;
	FILE *infile;
	int height, width, format;
	std::vector<psled::Pixel> imgIn;
	std::vector<psled::Pixel> imgOut;
	std::string filename;
	std::string filenamein;
	std::string filenameout;
	Pattern pattern;


	psled::Pixel grayscalePixel(psled::Pixel pixel){
		float Y = (0.299f * pixel.r) + (0.587 * pixel.g) + (0.114 * pixel.b);
		return  psled::Pixel (Y,Y,Y);
	}

	void hardcodeBunny() {
		width = 497;
		height = 373;
		filename = "owleye";
	}

	void test(/*std::string filename, int h, int w*/) {
		hardcodeBunny();
		char six[100];
		char *pSix = six;
		auto stencil = Stencil(IMG_PR_THREADCOUNT);



		// IMAGE READ
		// ------------------------------------------------------------------------------------

		std::cout << "READING IMAGE FILE:\n--------------------------------------------\n";
		filenamein = filename;
		filenamein.append(".ppm");
		printf(filenamein.c_str());
		printf("\n");
		infile = fopen(filenamein.c_str(), "r");

		std::cout << "\nReading pSix: \n";
		fscanf(infile, "%s\n", pSix);
		printf(pSix);
		printf("\n");

		// SOME IMGs REQUIRE THIS (example olweye)
		while (six[0] != '\n') {
			fscanf(infile, "%c", pSix);
			printf("%c", pSix[0]);
		}


		int he, wi, fo;
		fscanf(infile, "%d\n %d\n%d\n", &width, &height, &format);
		std::cout << "Size check -> (" << width << ", " << height << ", " << format << ")\n";
		imgIn = std::vector<psled::Pixel>(width * height);
		imgOut = std::vector<psled::Pixel>(width * height);

		std::cout << "Reading input colors\n";
		for (int i = 0; i < width; i++) {
			for (int j = 0; j < height; j++) {
				char c;
				 int k;
				fscanf(infile, "%c", &c);
				imgIn.at(i*height + j).r = (c + 256) % 256;

				fscanf(infile, "%c", &c);
				imgIn.at(i*height + j).g = (c + 256) % 256;

				fscanf(infile, "%c", &c);
				imgIn.at(i*height + j).b = (c + 256) % 256;

			}
		}

		std::cout << "closing files: IN\n";
		fclose(infile);


		// GRAYSCALING
		// --------------------------------------------------------------------------
		printf("GRAYSCALING\n");

		filenameout = filename;
		filenameout.append("_grayscale.ppm");

		outfile = fopen(filenameout.c_str(), "w");
		fprintf(outfile, "P6\n");
		fprintf(outfile, "%d %d\n%d\n", width, height, format);
		for (int i = 0; i < width; i++) {
			for (int j = 0; j < height; j++) {
				psled::Pixel px = grayscalePixel(imgIn.at(i*height + j));
				//imgIn.at(i*height + j) = px;
				//psled::Pixel px = (imgIn.at(i*height + j));

				fputc((char)(px.r), outfile);
				fputc((char)(px.g), outfile);
				fputc((char)(px.b), outfile);
			}
		}

		fclose (outfile);

		// BOX BLUR
		// -------------------------------------------------------------------------
		std::cout << "Box blurring ...\n";
		pattern = BoxBlur();
	 	stencil(imgOut, imgIn, pattern, PSLED_CROP, height, width);

	    // write img file
		filenameout = filename;
		filenameout.append("_box_blur.ppm");
		outfile = fopen(filenameout.c_str(), "w");
		fprintf(outfile, "P6\n");
		fprintf(outfile, "%d %d\n%d\n", width, height, format);
		for (size_t i = 0; i < width; i++) {
			for (int j = 0; j < height; j++) {
				fputc((char)(imgOut.at(i*height + j).r), outfile);
				fputc((char)(imgOut.at(i*height + j).g), outfile);
				fputc((char)(imgOut.at(i*height + j).b), outfile);
			}
		}
		// close files
		std::cout << "closing files: box blur\n";
		fclose(outfile);

		// GAUSSIAN BLUR 3x3
		// -------------------------------------------------------------------------
		std::cout << "Gaussian 3x3 blurring ...\n";
		pattern = GaussianBlur3x3();
		stencil(imgOut, imgIn, pattern, PSLED_CROP, height, width);

		// write img file
		filenameout = filename;
		filenameout.append("_gaus3_blur.ppm");
		outfile = fopen(filenameout.c_str(), "w");
		fprintf(outfile, "P6\n");
		fprintf(outfile, "%d %d\n%d\n", width, height, format);
		for (size_t i = 0; i < width; i++) {
			for (int j = 0; j < height; j++) {
				fputc((char)(imgOut.at(i*height + j).r), outfile);
				fputc((char)(imgOut.at(i*height + j).g), outfile);
				fputc((char)(imgOut.at(i*height + j).b), outfile);
			}
		}
		// close files
		std::cout << "closing files: gaus 3x3 blur\n";
		fclose(outfile);

		// GAUSSIAN 5x5 BLUR
		// -------------------------------------------------------------------------
		std::cout << "Gaussian 5x5 blurring ...\n";
		pattern = GaussianBlur5x5();

		stencil(imgOut, imgIn, pattern, PSLED_CROP, height, width);

		// write img file
		filenameout = filename;
		filenameout.append("_gaus5_blur.ppm");
		outfile = fopen(filenameout.c_str(), "w");
		fprintf(outfile, "P6\n");
		fprintf(outfile, "%d %d\n%d\n", width, height, format);
		for (size_t i = 0; i < width; i++) {
			for (int j = 0; j < height; j++) {
				fputc((char)(imgOut.at(i*height + j).r), outfile);
				fputc((char)(imgOut.at(i*height + j).g), outfile);
				fputc((char)(imgOut.at(i*height + j).b), outfile);
			}
		}
		// close files
		std::cout << "closing files: gaus 5x5 blur\n";
		fclose(outfile);

		// SHARP MASKING
		// -------------------------------------------------------------------------
		std::cout << "Sharpen mask ...\n";
		pattern = SharpenMask();
		stencil(imgOut, imgIn, pattern, PSLED_CROP, height, width);

		// write img file
		filenameout = filename;
		filenameout.append("_sharpen.ppm");
		outfile = fopen(filenameout.c_str(), "w");
		fprintf(outfile, "P6\n");
		fprintf(outfile, "%d %d\n%d\n", width, height, format);
		for (size_t i = 0; i < width; i++) {
			for (int j = 0; j < height; j++) {
				fputc((char)(imgOut.at(i*height + j).r), outfile);
				fputc((char)(imgOut.at(i*height + j).g), outfile);
				fputc((char)(imgOut.at(i*height + j).b), outfile);
			}
		}
		// close files
		std::cout << "closing files: sharpen mask\n";
		fclose(outfile);



		// EDGE DETECTION HARD
		// -------------------------------------------------------------------------
		std::cout << "Edge detection: hard ...\n";
		pattern = EdgeDetectionHard();
		stencil(imgOut, imgIn, pattern, PSLED_CROP, height, width);

		// write img file
		filenameout = filename;
		filenameout.append("_edgedetect_hard.ppm");
		outfile = fopen(filenameout.c_str(), "w");
		fprintf(outfile, "P6\n");
		fprintf(outfile, "%d %d\n%d\n", width, height, format);
		for (size_t i = 0; i < width; i++) {
			for (int j = 0; j < height; j++) {
				fputc((char)(imgOut.at(i*height + j).r), outfile);
				fputc((char)(imgOut.at(i*height + j).g), outfile);
				fputc((char)(imgOut.at(i*height + j).b), outfile);
			}
		}
		// close files
		std::cout << "closing files: edge detection: hard\n";
		fclose(outfile);

		// EDGE DETECTION MEDIUM
		// -------------------------------------------------------------------------
		std::cout << "Edge detection: medium ...\n";
		pattern = EdgeDetectionMedium();
		stencil(imgOut, imgIn, pattern, PSLED_CROP, height, width);

		// write img file
		filenameout = filename;
		filenameout.append("_edgedetect_medium.ppm");
		outfile = fopen(filenameout.c_str(), "w");
		fprintf(outfile, "P6\n");
		fprintf(outfile, "%d %d\n%d\n", width, height, format);
		for (size_t i = 0; i < width; i++) {
			for (int j = 0; j < height; j++) {
				fputc((char)(imgOut.at(i*height + j).r), outfile);
				fputc((char)(imgOut.at(i*height + j).g), outfile);
				fputc((char)(imgOut.at(i*height + j).b), outfile);
			}
		}
		// close files
		std::cout << "closing files: edge detection: medium\n";
		fclose(outfile);

		// EDGE DETECTION SOFT
		// -------------------------------------------------------------------------
		std::cout << "Edge detection: soft ...\n";
		pattern = EdgeDetectionSoft();
		stencil(imgOut, imgIn, pattern, PSLED_CROP, height, width);

		// write img file
		filenameout = filename;
		filenameout.append("_edgedetect_soft.ppm");
		outfile = fopen(filenameout.c_str(), "w");
		fprintf(outfile, "P6\n");
		fprintf(outfile, "%d %d\n%d\n", width, height, format);
		for (size_t i = 0; i < width; i++) {
			for (int j = 0; j < height; j++) {
				fputc((char)(imgOut.at(i*height + j).r), outfile);
				fputc((char)(imgOut.at(i*height + j).g), outfile);
				fputc((char)(imgOut.at(i*height + j).b), outfile);
			}
		}
		// close files
		std::cout << "closing files: edge detection: soft\n";
		fclose(outfile);


		// Emboss
		// -------------------------------------------------------------------------
		std::cout << "Emboss ...\n";
		pattern = Emboss();
		stencil(imgOut, imgIn, pattern, PSLED_CROP, height, width);

		// write img file
		filenameout = filename;
		filenameout.append("_emboss.ppm");
		outfile = fopen(filenameout.c_str(), "w");
		fprintf(outfile, "P6\n");
		fprintf(outfile, "%d %d\n%d\n", width, height, format);
		for (size_t i = 0; i < width; i++) {
			for (int j = 0; j < height; j++) {
				fputc((char)(imgOut.at(i*height + j).r), outfile);
				fputc((char)(imgOut.at(i*height + j).g), outfile);
				fputc((char)(imgOut.at(i*height + j).b), outfile);
			}
		}
		// close files
		std::cout << "closing files: emboss\n";
		fclose(outfile);


		std::cout << "finished";
	}
}

/*
		pat = BoxBlur();
		std::cout << " SIZE = " << boxblur.size() << "\n";
		for (int i = 0; i < pat.size(); i++) {
			std::cout << pat.rowOffset(i) << "\t" << pat.columnOffset(i) << "\t" << pat.itemWeight(i) << "\n";
		}
		pat = GaussianBlur3x3();
		std::cout << " SIZE = " << pat.size() << "\n";
		for (int i = 0; i < pat.size(); i++) {
			std::cout << pat.rowOffset(i) << "\t" << pat.columnOffset(i) << "\t" << pat.itemWeight(i) << "\n";
		}
		pat = GaussianBlur5x5();
		std::cout << " SIZE = " << pat.size() << "\n";
		for (int i = 0; i < pat.size(); i++) {
			std::cout << pat.rowOffset(i) << "\t" << pat.columnOffset(i) << "\t" << pat.itemWeight(i) << "\n";
		}
		pat = UnsharpMasking5x5();
		std::cout << " SIZE = " << pat.size() << "\n";
		for (int i = 0; i < pat.size(); i++) {
			std::cout << pat.rowOffset(i) << "\t" << pat.columnOffset(i) << "\t" << pat.itemWeight(i) << "\n";
		}
		pat = SharpenMask();
		std::cout << " SIZE = " << pat.size() << "\n";
		for (int i = 0; i < pat.size(); i++) {
			std::cout << pat.rowOffset(i) << "\t" << pat.columnOffset(i) << "\t" << pat.itemWeight(i) << "\n";
		}

		pat = EdgeDetectionHard();
		std::cout << " SIZE = " << pat.size() << "\n";
		for (int i = 0; i < pat.size(); i++) {
			std::cout << pat.rowOffset(i) << "\t" << pat.columnOffset(i) << "\t" << pat.itemWeight(i) << "\n";
		}

		pat = EdgeDetectionMedium();
		std::cout << " SIZE = " << pat.size() << "\n";
		for (int i = 0; i < pat.size(); i++) {
			std::cout << pat.rowOffset(i) << "\t" << pat.columnOffset(i) << "\t" << pat.itemWeight(i) << "\n";
		}

		pat = EdgeDetectionSoft();
		std::cout << " SIZE = " << pat.size() << "\n";
		for (int i = 0; i < pat.size(); i++) {
			std::cout << pat.rowOffset(i) << "\t" << pat.columnOffset(i) << "\t" << pat.itemWeight(i) << "\n";
		}
*/
#endif // !_TEST_IMAGE_PROCESSING_HPP
