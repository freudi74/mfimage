#include <UnitTest++.h>
#include <iostream> 
#include <stdexcept> 
#include "../src/Image.h"
#include "../src/ColorManager.h"

#include "folders.h"

#define TEST_FILES TESTDATA SEPARATOR "png_test_suite" SEPARATOR
#define TEST_OUT   TESTOUT  SEPARATOR "png" SEPARATOR


SUITE(PngSuite)
{
	void printPixelValues( const std::string & name, Image & in, Image & out )
	{
		PixelMode pm = in.getPixelMode();
		if ( pm.getBpc() == 16 )
		{
			if ( !pm.hasAlpha() )
			{
				Image::Pixel_GRAY_16* pixel1 = reinterpret_cast<Image::Pixel_GRAY_16*>( in.getPixel(3,8) );
				Image::Pixel_GRAY_16* pixel2 = reinterpret_cast<Image::Pixel_GRAY_16*>( out.getPixel(3,8) );
				std::cout << "Image: " << name << "pixel 3/8, in: " << pixel1->value << " out: " << pixel2->value << std::endl;
			}
		}
	}

	void readTransformWrite2( Image & img, const std::string & name, ColorManager & cms, cmsHPROFILE defaultGrayProfile, cmsHPROFILE defaultRgbProfile, cmsHPROFILE outProfile )
	{
		try {
			img.read( TEST_FILES + name + ".png" );
			img.removeAlphaChannel();
			// read profile from image, assigning defaults
//			img.convertTo8bpc();
			Image out;
			cmsHPROFILE defaultProfile = img.isGray() ? defaultGrayProfile : defaultRgbProfile;
			cmsHPROFILE imgProfile = defaultGrayProfile; // cms.readProfileFromImage( &img, defaultProfile );
			PixelMode newPM;
			if ( outProfile == nullptr )
			{
				outProfile = defaultProfile;
				newPM = img.getPixelMode();
				if ( img.hasAlpha() )
					out.copy( &img );
				else
					out.createFrom( &img );				
				}
			else
			{
				// hm, determine color mode of outProfile!
				// assume RGB for now...
				newPM = PixelMode::create( false, true, false, img.hasAlpha(), img.getPixelMode().getBpc() );
				out.create( img.getWidth(), img.getHeight(), newPM, img.getResolutionX(), img.getResolutionY() );
				if ( img.hasAlpha() )
					out.copyAlphaFrom( &img );
			}
#if 0
			RAIIcmsHTransform transformation( cms.createTransform( imgProfile, outProfile, INTENT_ABSOLUTE_COLORIMETRIC, img.getPixelMode().getLittleCmsType(), newPM.getLittleCmsType(), 0/*cmsFLAGS_BLACKPOINTCOMPENSATION*/|cmsFLAGS_HIGHRESPRECALC ) );

			for ( uint32_t iLine=0; iLine<img.getHeight(); iLine++ )
			{
				cmsDoTransform( transformation, img.getLine(iLine), out.getLine(iLine), img.getWidth() );
			}
#else
			out.copy( &img );
#endif
//			cms.embeddProfile( &out, outProfile );
//			printPixelValues( name, img, out );
//			out.write( TEST_OUT + name + ".tif", IE_TIFF );
			ImageCoderProperties props;
			props.compression = TIFF_Compression::LZMA;
			out.write( TEST_OUT + name + "_c.tif", IE_TIFF, &props );
		}
		catch ( std::runtime_error & e ) { \
			std::cout << "failed to read/write image '" << name << ": " << e.what() << std::endl; \
			CHECK( false ); \
		}
	}

	void readTransformWrite( Image & img, const std::string & name, ColorManager & cms, cmsHPROFILE defaultGrayProfile, cmsHPROFILE defaultRgbProfile )
	{
		readTransformWrite2( img, name, cms, defaultGrayProfile, defaultRgbProfile, nullptr );
	}

	
	/* this suite tests the PNG implementation by using reference files from www.schaik.com */
	TEST(basic)
	{
		START_TEST();
		ColorManager cms;
		cmsHPROFILE defaultGray = cms.createGrayProfile( &ColorManager::WHITE_POINT_D65, 1.0 );
		cmsHPROFILE defaultRGB  = cms.createSrgbProfile();
/*			cmsCIExyY d50;
		cmsCIExyY d65;
		cmsCIExyY d70;
		cmsCIExyY d75;
		cmsWhitePointFromTemp( &d50, 5004 );
		cmsWhitePointFromTemp( &d65, 6504 );
		cmsWhitePointFromTemp( &d70, 7004 );
		cmsWhitePointFromTemp( &d75, 7504 );
*/		
		Image img; 
/*		readTransformWrite( img, "basn0g01", cms, defaultGray, defaultRGB );
		readTransformWrite( img, "basn0g02", cms, defaultGray, defaultRGB );
		readTransformWrite( img, "basn0g04", cms, defaultGray, defaultRGB );
		readTransformWrite( img, "basn0g08", cms, defaultGray, defaultRGB );
		readTransformWrite( img, "basn0g16", cms, defaultGray, defaultRGB );
		readTransformWrite( img, "basn2c08", cms, defaultGray, defaultRGB );
		readTransformWrite( img, "basn2c16", cms, defaultGray, defaultRGB );
		readTransformWrite( img, "basn3p01", cms, defaultGray, defaultRGB );
		readTransformWrite( img, "basn3p02", cms, defaultGray, defaultRGB );
		readTransformWrite( img, "basn3p04", cms, defaultGray, defaultRGB );
		readTransformWrite( img, "basn3p08", cms, defaultGray, defaultRGB );
*/		

			readTransformWrite2( img, "basn4a08", cms, defaultGray, defaultRGB, defaultRGB );
			readTransformWrite( img, "basn4a16", cms, defaultGray, defaultRGB );
		
/*		readTransformWrite( img, "basn6a08", cms, defaultGray, defaultRGB );
		readTransformWrite( img, "basn6a16", cms, defaultGray, defaultRGB );
}
*/
//		cmsCloseProfile( defaultGray ); // fails for obscure reasons. Seems to try to store the file. lcms2.6 bug?
//		cmsCloseProfile( defaultRGB ); // fails for obscure reasons. Seems to try to store the file. lcms2.6 bug?
		END_TEST();
	}
	
	TEST(gamma)
	{
		START_TEST();
		ColorManager cms;
//		RAIIcmsHProfile defaultGray( cms.createGrayProfile( &ColorManager::WHITE_POINT_D50, 2.2 ) );
//		RAIIcmsHProfile defaultRGB( cms.createSrgbProfile() );
		Image img; 
/*		readTransformWrite( img, "g03n0g16", cms, defaultGray, defaultRGB ); // g03n0g16 - grayscale, file-gamma = 0.35
		CHECK( img.isGray() );
		CHECK( !img.hasAlpha() );
		readTransformWrite( img, "g03n2c08", cms, defaultGray, defaultRGB ); // g03n2c08 - color, file-gamma = 0.35
		CHECK( img.isRGB() );
		CHECK( !img.hasAlpha() );
		readTransformWrite( img, "g03n3p04", cms, defaultGray, defaultRGB ); // g03n3p04 - paletted, file-gamma = 0.35
		CHECK( img.isRGB() );
		CHECK( !img.hasAlpha() );
		readTransformWrite( img, "g04n0g16", cms, defaultGray, defaultRGB ); // g04n0g16 - grayscale, file-gamma = 0.45
		CHECK( img.isGray() );
		CHECK( !img.hasAlpha() );
		readTransformWrite( img, "basn0g16", cms, defaultGray, defaultRGB ); // 
		readTransformWrite( img, "basn2c08", cms, defaultGray, defaultRGB ); // 
		readTransformWrite( img, "g05n0g16", cms, defaultGray, defaultRGB ); // g05n0g16 - grayscale, file-gamma = 0.55
		CHECK( img.isGray() );
		CHECK( !img.hasAlpha() );
		readTransformWrite( img, "basn3p01", cms, defaultGray, defaultRGB ); // 
		readTransformWrite( img, "basn3p02", cms, defaultGray, defaultRGB ); // 
		readTransformWrite( img, "g07n0g16", cms, defaultGray, defaultRGB ); // g07n0g16 - grayscale, file-gamma = 0.70
		CHECK( img.isGray() );
		CHECK( !img.hasAlpha() );
		readTransformWrite( img, "basn3p08", cms, defaultGray, defaultRGB ); // 
		readTransformWrite( img, "basn4a08", cms, defaultGray, defaultRGB ); // 
*/	//	readTransformWrite( img, "g10n0g16", cms, defaultGray, defaultRGB ); // g10n0g16 - grayscale, file-gamma = 1.00
	//	CHECK( img.isGray() );
	//	CHECK( !img.hasAlpha() );
//		readTransformWrite( img, "basn6a08", cms, defaultGray, defaultRGB ); // 
//		readTransformWrite( img, "basn6a16", cms, defaultGray, defaultRGB ); // 
	//	readTransformWrite( img, "g25n0g16", cms, defaultGray, defaultRGB ); // g25n0g16 - grayscale, file-gamma = 2.50
	//	CHECK( img.isGray() );
	//	CHECK( !img.hasAlpha() );
		 	
/*		 	
		 	g04n2c08 - color, file-gamma = 0.45
		 	g04n3p04 - paletted, file-gamma = 0.45
		 	
		 	g05n2c08 - color, file-gamma = 0.55
	 	 	g05n3p04 - paletted, file-gamma = 0.55
	 	 	
		 	g07n2c08 - color, file-gamma = 0.70
		 	g07n3p04 - paletted, file-gamma = 0.70
		 	
		 	g10n2c08 - color, file-gamma = 1.00
		 	g10n3p04 - paletted, file-gamma = 1.00
		 	
		 	g25n2c08 - color, file-gamma = 2.50
		 	g25n3p04 - paletted, file-gamma = 2.50 		
*/
		END_TEST();
	}
}
