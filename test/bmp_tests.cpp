#include <UnitTest++/UnitTest++.h>
#include <list> 
#include <string> 
#include <iostream>
#include "../src/Image.h"
#include "folders.h"

#ifndef SKIP_BMP_SUITE

#define TEST_FILES TESTDATA SEPARATOR "bmp" SEPARATOR
#define TEST_OUT   TESTOUT  SEPARATOR "bmp" SEPARATOR


SUITE(BmpSuite)
{
	TEST(bmp)
	{
		START_TEST();
		std::list<std::string> testFileNames = { 
			"alex8bit.bmp",
			"DadWood.bmp",
			"12x12 Women sample 400 dpi.bmp",
			"12x12 women sample 400 dpi,black marble.bmp",
			"ffc.bmp",
			"B1B.BMP",
			"TREES.BMP",
			"Z_BUZZIN.BMP",
			"CALORIES.BMP"
//			"../jp2/cmyk.jp2"
//			"gray-normal.jpg"
		};
		Image img; 
		for ( std::string & testFileName : testFileNames )
		{
			std::cout << "file: " << testFileName << std::endl;
			img.read( TEST_FILES + testFileName, 1 );
			img.write( TEST_OUT + testFileName + ".tif", IE_TIFF );
		}			
		END_TEST();			
	}
	
	TEST(bmp_alpha)
	{
		START_TEST();
		std::list<std::string> testFileNames = { 
			"rgba32.bmp"
		};
		Image img; 
		for ( std::string & testFileName : testFileNames )
		{
			std::cout << "file: " << testFileName << std::endl;
			img.read( TEST_FILES + testFileName, 1 );
			img.write( TEST_OUT + testFileName + ".tif", IE_TIFF );
			img.write( TEST_OUT + testFileName + ".bmp", IE_BMP );
		}
		END_TEST();			
	}	
	
	TEST(bmp_rle)
	{
		START_TEST();
		createFolder( TEST_OUT );
		std::list<std::string> testFileNames = { 
//			"testcompress4.bmp",
			"testcompress8.bmp"
		};
		Image img; 
		for ( std::string & testFileName : testFileNames )
		{
			std::cout << "file: " << testFileName << std::endl;
			img.read( TEST_FILES + testFileName, 1 );
			img.write( TEST_OUT + testFileName + ".tif", IE_TIFF );
		}			
		END_TEST();
	}
	
	TEST(bmp_embedded_profile) 
	{
		START_TEST();
		createFolder( TEST_OUT );
		std::list<std::string> testFileNames = { 
//			"testcompress4.bmp",
			"rgb24prof.bmp"
		};
		Image img; 
		for ( std::string & testFileName : testFileNames )
		{
			std::cout << "file: " << testFileName << std::endl;
			img.read( TEST_FILES + testFileName, 1 );
			img.write( TEST_OUT + testFileName + ".tif", IE_TIFF );
			img.write( TEST_OUT + testFileName + ".bmp", IE_BMP );
		}			
		END_TEST();
	}

 }

#endif