#include <unittest++/UnitTest++.h>
#include <iostream> 
#include <stdexcept> 
#include <list>
#include "../src/Image.h"
#include "../src/ColorManager.h"

#include "folders.h"

#ifndef SKIP_TIFF_SUITE

#define TEST_1_FILES TESTDATA SEPARATOR "tiff" SEPARATOR
#define TEST_1_OUT   TESTOUT  SEPARATOR "tiff" SEPARATOR

#define TEST_2_FILES TESTDATA SEPARATOR "libtiffpic" SEPARATOR
#define TEST_2_OUT   TESTOUT  SEPARATOR "libtiffpic" SEPARATOR

SUITE(TiffSuite)
{
	///////////////////////
	// marcos own stuff
	///////////////////////

	TEST(multipage)
	{
		START_TEST();
		std::list<std::string> testFileNames = { "multipage_tiff_example.tif", "multipage_tiff_example_2.tif" };
		Image img; 
		for ( std::string & testFileName : testFileNames )
		{
			for ( int i=1; i<=10; i++ )
			{
				img.read( TEST_1_FILES + testFileName, i );
				img.write( TEST_1_OUT + testFileName + "-" + std::to_string(i) + ".bmp", IE_BMP );
			}
		}
		END_TEST();
	}
	
	TEST(AlphaRGB)
	{
		START_TEST();
		std::list<std::string> testFileNames = { 
			"transbugs-rgb-a-8-contig.tif",
			"Test_Green_with_transparent_ARGB.tif"

		};
		Image img; 
		for ( std::string & testFileName : testFileNames )
		{
			img.read( TEST_1_FILES + testFileName, 1 );
			img.write( TEST_1_OUT + testFileName + ".tif", IE_TIFF );
		}			
		END_TEST();
	}
	 
	/////////////////////////
	// Stuff from libtiff
	/////////////////////////
	TEST(Gray)
	{
		START_TEST();
		std::list<std::string> testFileNames = { 
			"fax2d.tif",    // 1728x1082 1-bit b&w (G3/2D) facsimile - miniswhite, fillorder = 2;
			"cramps.tif",   // 800x607 8-bit b&w (packbits) "cramps poster" - miniswhite, fillorder = 1;
			"g3test.tif",   // raw Group 3 encoded fax file as TIFF created by fax2tiff
//			"text.tif",     // -- this is actually a 2-page file! here we only test page 1 --- Theres a problem with that file!
			"jim___cg.tif", // halftonehint-test
			"jim___dg.tif", // halftonehint-test
			"jim___gg.tif"  // halftonehint-test
		};
		Image img; 
		for ( std::string & testFileName : testFileNames )
		{
			img.read( TEST_2_FILES + testFileName, 1 );
			img.write( TEST_2_OUT + testFileName + ".tif", IE_TIFF );
		}		
		END_TEST();
	}

	TEST(Tiles_Contig)
	{
		START_TEST();
		std::list<std::string> testFileNames = { 
			"cramps.tif", // stripes --- just for reference, should look the same as cramps-tile.tif
			"cramps-tile.tif" // tiles 
			};
		Image img; 
		for ( std::string & testFileName : testFileNames )
		{
			img.read( TEST_2_FILES + testFileName, 1 );
			img.write( TEST_2_OUT + testFileName + ".bmp", IE_BMP );
		}		
		END_TEST();
	}

	TEST(Tiles_Separated)
	{
		START_TEST();
		std::list<std::string> testFileNames = { 
			"tiger-rgb-tile-contig-08.tif", 
			"tiger-rgb-tile-contig-16.tif", 
			"tiger-rgb-tile-planar-08.tif", 
			"tiger-rgb-tile-planar-16.tif" 
		};
		Image img; 
		for ( std::string & testFileName : testFileNames )
		{
			img.read( TEST_2_FILES "tiger" SEPARATOR + testFileName, 1 );
			img.write( TEST_2_OUT  "tiger" SEPARATOR + testFileName + ".tif", IE_TIFF );
		}		
		END_TEST();
	}

	////////////////////////
	// External stuff from libtiff (actually GraphicsMagick)
	////////////////////////
	TEST(DepthGray)
	{
		START_TEST();
		// we don't support 2, 6, 10, 12, 14, 24, 32 bpc RGB !
		std::list<std::string> testFileNames = { 
//			"flower-minisblack-02.tif",
			"flower-minisblack-04.tif",
//			"flower-minisblack-06.tif",
			"flower-minisblack-08.tif",
//			"flower-minisblack-10.tif",
//			"flower-minisblack-12.tif",
//			"flower-minisblack-14.tif",
			"flower-minisblack-16.tif",
//			"flower-minisblack-24.tif",
//			"flower-minisblack-32.tif",
		};
		Image img; 
		for ( std::string & testFileName : testFileNames )
		{
			img.read( TEST_2_FILES "depth" SEPARATOR + testFileName, 1 );
			img.write( TEST_2_OUT "depth" SEPARATOR + testFileName + ".tif", IE_TIFF );
		}			
		END_TEST();
	}
	
	TEST(DepthPalette)
	{
		START_TEST();
		// we don't support 2, 16 bpc palettes!
		std::list<std::string> testFileNames = { 
//			"flower-palette-02.tif",
			"flower-palette-04.tif",
			"flower-palette-08.tif",
//			"flower-palette-16.tif",
		};
		Image img; 
		for ( std::string & testFileName : testFileNames )
		{
			img.read( TEST_2_FILES "depth" SEPARATOR + testFileName, 1 );
			img.write( TEST_2_OUT "depth" SEPARATOR + testFileName + ".tif", IE_TIFF );
		}		
		END_TEST();
	}

	TEST(DepthRGB)
	{
		START_TEST();
		// we don't support 4, 10, 12, 14, 24, 32 bpc RGB !
		std::list<std::string> testFileNames = { 
//			"flower-rgb-contig-04.tif", 
			"flower-rgb-contig-08.tif",
//			"flower-rgb-contig-10.tif",
//			"flower-rgb-contig-12.tif",
//			"flower-rgb-contig-14.tif",
			"flower-rgb-contig-16.tif",
//			"flower-rgb-contig-24.tif",
//			"flower-rgb-contig-32.tif",
//			"flower-rgb-planar-04.tif",
			"flower-rgb-planar-08.tif",
//			"flower-rgb-planar-10.tif",
//			"flower-rgb-planar-12.tif",
//			"flower-rgb-planar-14.tif",
			"flower-rgb-planar-16.tif"
//			"flower-rgb-planar-24.tif",
//			"flower-rgb-planar-32.tif",
		};
		Image img; 
		for ( std::string & testFileName : testFileNames )
		{
			img.read( TEST_2_FILES "depth" SEPARATOR + testFileName, 1 );
			img.write( TEST_2_OUT "depth" SEPARATOR + testFileName + ".tif", IE_TIFF );
		}		
		END_TEST();
	}
	
	TEST(DepthCMYK)
	{
		START_TEST();
		std::list<std::string> testFileNames = { 
			"flower-separated-contig-08.tif", 
			"flower-separated-contig-16.tif", 
			"flower-separated-planar-08.tif", 
			"flower-separated-planar-16.tif" 
		};

		for ( std::string & testFileName : testFileNames )
		{
			Image img;
			img.read( TEST_2_FILES "depth" SEPARATOR + testFileName, 1 );
			img.write( TEST_2_OUT "depth" SEPARATOR + testFileName + ".tif", IE_TIFF );
		}		
		END_TEST();	
	}
	
	TEST(Tiff_FileFormat_info)
	{
		START_TEST();
		std::list<std::string> testFileNames = { 
			"CCITT_1.TIF", 
			"CCITT_2.TIF", 
			"CCITT_3.TIF", 
			"CCITT_4.TIF", 
			"CCITT_5.TIF", 
			"CCITT_6.TIF", 
			"CCITT_7.TIF", 
			"CCITT_8.TIF", 
			"FLAG_T24.TIF", 
			"G31D.TIF", 
			"G31DS.TIF", 
			"G32D.TIF", 
			"G32DS.TIF", 
			"G4.TIF", 
			"G4S.TIF", 
			"GMARBLES.TIF", 
			"MARBIBM.TIF", 
			"MARBLES.TIF", 
			"XING_T24.TIF" 
		};
		Image img; 
		for ( std::string & testFileName : testFileNames )
		{
			img.read( TEST_1_FILES "ff" SEPARATOR + testFileName, 1 );
			img.write( TEST_1_OUT "ff" SEPARATOR + testFileName + ".tif", IE_TIFF );
		}		
		END_TEST();
	}
	
	TEST(WritePlanar)
	{
		START_TEST();
		std::list<std::string> testFileNames = { 
			"flower-separated-contig-08.tif", 
			"flower-separated-contig-16.tif", 
			"flower-rgb-contig-08.tif",
			"flower-rgb-contig-16.tif",
		};

		ImageCoderProperties props;
		props.writeSeparatedPlanes = true;
		for ( std::string & testFileName : testFileNames )
		{
			Image img;
			img.read( TEST_2_FILES "depth" SEPARATOR + testFileName, 1 );
			img.write( TEST_2_OUT "depth" SEPARATOR + testFileName + "-toPlanar.tif", IE_TIFF, &props );
		}		
		END_TEST();	
	}
}

#endif