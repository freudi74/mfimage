#include <unittest++/UnitTest++.h>
#include <list> 
#include <string> 
#include "../src/Image.h"
#include "folders.h"

#ifndef SKIP_JPEG_SUITE

#define TEST_FILES TESTDATA SEPARATOR "jpg" SEPARATOR
#define TEST_OUT   TESTOUT  SEPARATOR "jpg" SEPARATOR

SUITE(JpegSuite)
{
	TEST(jpeg_gray) 
	{
		START_TEST();
		std::list<std::string> testFileNames = { 
			"Channel_digital_image_green.jpg",
			"gray-normal.jpg"
		};
		Image img; 
		for ( std::string & testFileName : testFileNames )
		{
			img.read( TEST_FILES + testFileName, 1 );
			img.write( TEST_OUT + testFileName + ".tif", IE_TIFF );
		}			
		END_TEST();
	}

	TEST(JpegRGB)
	{
		START_TEST();
		Image img;
		img.read( TEST_FILES "gray-jpg.jpg" );
		img.write( TEST_OUT "gray-jpg.tif", IE_TIFF );
		// compare results against a reference?
		END_TEST();
	}

	TEST(jpeg_rgb) 
	{
		START_TEST();
		std::list<std::string> testFileNames = { 
//			"MARBLES.PCX",
			"rgb-normal.jpg"
		};
		Image img; 
		for ( std::string & testFileName : testFileNames )
		{
			img.read( TEST_FILES + testFileName, 1 );
			img.write( TEST_OUT + testFileName + ".tif", IE_TIFF );
		}			
		END_TEST();
	}

	TEST(jpeg_cmyk) 
	{
		START_TEST();
		std::list<std::string> testFileNames = { 
			"Channel_digital_image_CMYK_color.jpg",
//			"3eebcd76-9d29-4970-9156-5f126f38dc7e.jpg"
		};
		Image img; 
		for ( std::string & testFileName : testFileNames )
		{
			img.read( TEST_FILES + testFileName, 1 );
			img.write( TEST_OUT + testFileName + ".tif", IE_TIFF );
		}			
		END_TEST();
	}
/*	
	TEST(DC_EAR)
	{
		std::list<std::string> testFileNames = { 
			"LHLEFAHCI2_BAD.jpg",
//			"3eebcd76-9d29-4970-9156-5f126f38dc7e.jpg"
		};
		Image img; 
		for ( std::string & testFileName : testFileNames )
		{
			img.read( TEST_FILES + testFileName, 1 );
			img.write( TEST_OUT + testFileName + ".tif", IE_TIFF );
			int i=0;
			std::vector<Image> seps = img.separate();
			for ( Image & sep : seps) 
			{
				sep.write( TEST_OUT + testFileName + "-sep-" + std::to_string(++i) + ".tif", IE_TIFF );
			}
			std::vector<Image> newSrc(3);
			newSrc[0].copy(&seps[0]);
			newSrc[1].copy(&seps[1]);
			newSrc[2].copy(&seps[2]);
			Image RGBfromCMY;
			RGBfromCMY.combine( PixelMode::RGB8, newSrc );
			RGBfromCMY.write( TEST_OUT + testFileName + "-recombined.tif", IE_TIFF );

			Image out;
			img.flip( Image::FlipDirection::Vertical, &out );
			out.write( TEST_OUT + testFileName + "_fv.tif", IE_TIFF );
			img.flip( Image::FlipDirection::Horizontal, &out );
			out.write( TEST_OUT + testFileName + "_fh.tif", IE_TIFF );
			img.flip( Image::FlipDirection::SwapXY, &out );
			out.write( TEST_OUT + testFileName + "_sxy.tif", IE_TIFF );
			Image out2;
			out.flip( Image::FlipDirection::Vertical, &out2 );
			out2.write( TEST_OUT + testFileName + "_r90ccw.tif", IE_TIFF );
			out.flip( Image::FlipDirection::Horizontal, &out2 );
			out2.write( TEST_OUT + testFileName + "_r90cw.tif", IE_TIFF );
			img.flip( Image::FlipDirection::Horizontal, &out );
			out.flip( Image::FlipDirection::Vertical, &out2 );
			out2.write( TEST_OUT + testFileName + "_r180.tif", IE_TIFF );
			
			
			
		}			
	}
*/
}

#endif