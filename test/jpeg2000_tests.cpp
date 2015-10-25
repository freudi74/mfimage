#include <UnitTest++.h>
#include <list> 
#include <string> 
#include <iostream>
#include "../src/Image.h"

#include "folders.h"

#define TEST_FILES TESTDATA SEPARATOR "jp2" SEPARATOR
#define TEST_OUT   TESTOUT  SEPARATOR "jp2" SEPARATOR


SUITE(Jpeg2000Suite)
{
	TEST(jp2)
	{
		START_TEST();
		std::list<std::string> testFileNames = { 
			"file1.jp2", 
			"relax.jp2",
			"1.jp2",
			"balloon.jp2"
		};
		Image img; 
		for ( int i=0; i<50; i++ )
		for ( std::string & testFileName : testFileNames )
		{
			img.read( TEST_FILES + testFileName, 1 );
			img.write( TEST_OUT + testFileName + ".tif", IE_TIFF );
		}
		END_TEST()
	}
}

