#include <UnitTest++/UnitTest++.h>
#include <list> 
#include <string> 
#include "../src/Image.h"

#include "folders.h"

#ifndef SKIP_PCX_SUITE

#define TEST_FILES TESTDATA SEPARATOR "pcx" SEPARATOR
#define TEST_OUT   TESTOUT  SEPARATOR "pcx" SEPARATOR

SUITE(PcxSuite)
{
	TEST(pcx_color_8bit) 
	{
		START_TEST();
		std::list<std::string> testFileNames = { 
//			"MARBLES.PCX",
			"yolinux-mime-test.pcx"
		};
		Image img; 
		for ( std::string & testFileName : testFileNames )
		{
			img.read( TEST_FILES + testFileName, 1 );
			img.write( TEST_OUT + testFileName + ".tif", IE_TIFF );
		}			
		END_TEST();
	}
	/* this suite tests the PNG implementation by using reference files from www.schaik.com */
	TEST(pcx_gray)
	{
		START_TEST();
		std::list<std::string> testFileNames = { 
//			"GMARBLES.PCX" // note that GMARBLES file is actually bad. The PCX stream ends prematurely after ~ 998 lines
		};
		Image img; 
		for ( std::string & testFileName : testFileNames )
		{
			img.read( TEST_FILES + testFileName, 1 );
			img.write( TEST_OUT + testFileName + ".tif", IE_TIFF );
		}			
		END_TEST();
	}
}

#endif