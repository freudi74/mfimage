/* ImageCoderPng.cpp
 * Copyright 2015 Marco Freudenberger.
 * 
 * This file is part of mfimage library.
 * 
 * Design and implementation by:
 * --------------------------------------------------
 * - Marco Freudenberger (Marco.Freudenberger@gmx.de)
 *
 */
 
#include "stdafx.h"
#include "ImageCoderPng.h"

#include <png.h>
#include "ColorManager.h"
#include "TimeStamper.h"

#define READ_SINGLE_ROWS 0	/* faster ? But why ?  FAILS FOR INTERLACED IMAGES !!!!*/
// #define ASSUME_CCIR_709 1 /* assign CCIR_709 if no cHRM is supplied */

/* default whitepoint and primaries (CCIR 709 / Rec. 709 / HDTV) in case no cHRM (chroma) values are supplied.
 * Rec. 709 has the same primaries as sRGB. Linearization function is slightly different, but we use fixed gAMA anyways.
 */
static const cmsCIExyY       CCIR_709_WHITEPOINT = { 0.3127, 0.3290, 1.0 };
//static const cmsCIExyYTRIPLE CCIR_709_PRIMARIES = { { 0.640, 0.330, 0.2126 }, { 0.300, 0.600, 0.7152 }, { 0.150, 0.060, 0.0722 } };
static const cmsCIExyYTRIPLE CCIR_709_PRIMARIES = { { 0.640, 0.330, 1.0 }, { 0.300, 0.600, 1.0 }, { 0.150, 0.060, 1.0 } };
static const double          DEFAULT_FILE_GAMMA = 0.45455; // = 1/2.2 (roughly, same accuracy as PNG, close enough)

ImageCoderPng::ImageCoderPng(Image* img) : ImageCoder(IE_PNG, img)
{
}


ImageCoderPng::~ImageCoderPng()
{
}

bool ImageCoderPng::canEncode()
{
	return false;	
}

bool ImageCoderPng::canDecode()
{
	return true;
}
bool ImageCoderPng::canEncode( PixelMode pixelMode )
{
	return false;
}

#if 0
static int unkownChunkCallback( png_structp png, png_unknown_chunkp chunk)
{
	std::string name(reinterpret_cast<char*>(chunk->name));
	if (name == "cHRM")
	{
		return 0;	// success handling chunk. chunk had an error ...
	}
	return 0;	// unrecognized chunk
}
#endif

static void readData(png_structp pngPtr, png_bytep data, png_size_t length)
{
	ImageCoderPng* pThis = reinterpret_cast<ImageCoderPng*>( png_get_io_ptr(pngPtr) );
	if (pThis->_dataLeft < length)
		png_error(pngPtr, "end of stream reached.");
	memcpy(data, pThis->_dataCur, length);
	pThis->_dataLeft -= length;
	pThis->_dataCur += length;
}

void ImageCoderPng::read(const std::string & filename)
{
	std::ifstream file(filename.c_str(), std::ios_base::binary);
	if (file.fail())
		throw std::runtime_error("failed to open PNG file " + filename);

	read(file);
}


void ImageCoderPng::read(std::istream & stream)
{
	auto errorHandler = [](png_structp png, png_const_charp errorMsg) {
		throw std::runtime_error(errorMsg);
	};
	auto warningHandler = [](png_structp png, png_const_charp warningMsg) {
		throw std::runtime_error(warningMsg);
	};

	TimeStamper ts(9);
	ts.setStamp(0);

	png_structp png = png_create_read_struct( PNG_LIBPNG_VER_STRING, nullptr /*(png_voidp)user_error_ptr*/, errorHandler,  warningHandler );
	if (!png)
		throw std::runtime_error("Failed to create PNG read structure");

	// there's a big discussion about the fact that libpng checks for "broken" sRGB profiles, which aren't really broken.
	// turn that check off (skip check)
#if defined(PNG_SKIP_sRGB_CHECK_PROFILE) && defined(PNG_SET_OPTION_SUPPORTED)
	png_set_option(png, PNG_SKIP_sRGB_CHECK_PROFILE, PNG_OPTION_ON);
#endif

	png_infop pngInfo = nullptr, endInfo = nullptr;
	try
	{
		pngInfo = png_create_info_struct(png);
		if (! pngInfo)
			throw std::runtime_error("Failed to create PNG info structure");
	
		endInfo = png_create_info_struct(png);
		if (! endInfo)
			throw std::runtime_error("Failed to create PNG end info structure");


#if 0
		png_init_io(png, pFile);
#else
		// for now, we read all the data at once to a buffer and work from there.
		// only good for in-memory and files, not good for network files or such.

		// get length of file:
		stream.seekg(0, stream.end);
		if (stream.fail())
			throw std::runtime_error("Failed to get size of PNG file");
		std::streamoff length = stream.tellg();
		stream.seekg(0, stream.beg);
		if (stream.fail())
			throw std::runtime_error("Failed to get size of PNG file");

		_data.reset(new char[length]);
		_dataCur = _data.get();
		_dataLeft = static_cast<size_t>(length);

		stream.read(_data.get(), _dataLeft);
		if (stream.fail())
			throw std::runtime_error("Failed to read PNG file");

		png_set_read_fn(png, this, readData);
#endif


		ts.setStamp(1);

		// png_set_read_user_chunk_fn(png, nullptr, unkownChunkCallback );
		// png_set_read_status_fn(png_ptr, read_row_callback);	read_row_callback := void (png_ptr ptr, png_uint_32 row, int pass);

		png_read_info(png, pngInfo );

		ts.setStamp(2);

		png_uint_32 width, height;
		int bitDepth, colorType, interlaceMethod, compressionMethod, filterMethod;
		uint32_t res;

		res = png_get_IHDR(png, pngInfo, &width, &height, &bitDepth, &colorType, &interlaceMethod, &compressionMethod, &filterMethod);
		if (res == 0)
			throw std::runtime_error("Failed reading the PNG IHDR");
		
		// read DPIs if present
		float dpiX = 72.0f, dpiY = 72.0f;
		{
			png_uint_32 resX, resY;
			int unitType;
			res = png_get_pHYs(png, pngInfo, &resX, &resY, &unitType);
			if (res > 1 && unitType == PNG_RESOLUTION_METER)
			{
				dpiX = static_cast<float>(resX) * 0.0254f;
				dpiY = static_cast<float>(resY) * 0.0254f;
			}
		}

		if (interlaceMethod == PNG_INTERLACE_ADAM7)
		{
			png_set_interlace_handling(png);
		}

		// set up the image...
		PixelMode pm;
		bool hasColorTransparency = 0 != png_get_valid(png, pngInfo, PNG_INFO_tRNS); // there is non-alpha transparency information... we want this as alpha!;
		switch (colorType)
		{
		case PNG_COLOR_TYPE_RGB:
			if (hasColorTransparency)
			{
				pm = (bitDepth == 16) ? PixelMode::ARGB16 : PixelMode::ARGB8;
				png_set_tRNS_to_alpha(png);	// transform tRNS to alpha
				png_set_swap_alpha(png);    // ARGB instead of RGBA
			}
			else
				pm = (bitDepth == 16) ? PixelMode::RGB16 : PixelMode::RGB8;
			break;

		case PNG_COLOR_TYPE_RGB_ALPHA:
			pm = (bitDepth == 16) ? PixelMode::ARGB16 : PixelMode::ARGB8;
			png_set_swap_alpha(png);	// ARGB instead of RGBA
			if (hasColorTransparency)
				png_set_tRNS_to_alpha(png);
			break;
		
		case PNG_COLOR_TYPE_PALETTE:
			png_set_palette_to_rgb(png);	// transform palette to RGB color automatically
			if (hasColorTransparency)
			{
				pm = PixelMode::ARGB8;
				png_set_tRNS_to_alpha(png);	// transform tRNS to alpha
				png_set_swap_alpha(png);    // ARGB instead of RGBA
			}
			else
			{
				pm = PixelMode::RGB8;
			}
			if (bitDepth < 8)
				png_set_expand_gray_1_2_4_to_8(png);	// transform 1,2,4 bpp to 8 bpp
			break;
		
		case PNG_COLOR_TYPE_GRAY:
			if (hasColorTransparency)
			{
				pm = (bitDepth == 16) ? PixelMode::AGRAY16 : PixelMode::AGRAY8;
				png_set_tRNS_to_alpha(png);	// transform tRNS to alpha
				png_set_swap_alpha(png);  // AG instead of GA
			}
			else
			{
				pm = (bitDepth == 16) ? PixelMode::GRAY16 : PixelMode::GRAY8;
			}
			if (bitDepth < 8)
				png_set_expand_gray_1_2_4_to_8(png);	// transform 1,2,4 bpp to 8 bpp
			break;
			
		case PNG_COLOR_TYPE_GRAY_ALPHA:
			pm = (bitDepth == 16) ? PixelMode::AGRAY16 : PixelMode::AGRAY8;
			png_set_swap_alpha(png);  // AG instead of GA
			break;
		}

		if (bitDepth == 16)
			png_set_swap(png);	// swich to little endian ...

		//int numberOfPasses =  png_set_interlace_handling(png);

		ts.setStamp(3);

		// create our image !
		image->create(width, height, pm, dpiX, dpiY);

		ts.setStamp(4);

		//////////////////////////////////////////////////////////////////////////
		// Color Management
		// Priority:
		// - iCCP chunk, if present, if used (image has embedded ICC)
		// - sRGB chunk, if present, is used (image is sRGB)
		// - cHRM + gAMA chunk, if one present, are used
		//    - if cHRM is present without gAMA, assume gAMA 0.45455 (1/2.2)
		//    - if gAMA is present without cHRM, assume cHRM of Rec. 709 (same as sRGB) / gray D50/(1/gamma)
		// - if non is present, assume sRGB / gray D50/2.2 ????
		//////////////////////////////////////////////////////////////////////////
		bool hasIcc = false;
		bool issRgb = false;
		bool hasGamma = false;
		bool hasChroma = false;
		getIccProfile().clear();
		{
			// read icc profile if present
			png_charp name;
      // Confusion in libpng. Up to 1.4, "profile" was png_charp, since 1.5 it's png_bytep
#if ( PNG_LIBPNG_VER_MAJOR>1 || (PNG_LIBPNG_VER_MAJOR==1 && PNG_LIBPNG_VER_MINOR>=5) )
			png_bytep profile;
#else
			png_charp profile;
#endif
			png_uint_32 profileLen;
			int compressionType;
			res = png_get_iCCP(png, pngInfo, &name, &compressionType, &profile, &profileLen);
			if (res > 0 && profileLen > 0)
			{
				hasIcc = true;
				std::string profileName(name);
				getIccProfile().assign(reinterpret_cast<char*>(profile), profileLen);
			}
		}
		if (!hasIcc)
		{
			// no icc - check the alternatives ...

			// check if it is supposed to be sRGB (sRGB chunk present - contains rendering intent)
			int srgbIntent = -1;
			res = png_get_sRGB(png, pngInfo, &srgbIntent);
			if (res > 0 && srgbIntent >= 0 && srgbIntent <= 3)
				issRgb = true;
			
			// is a gamma and chroma supplied ?
			double fileGamma;
			res = png_get_gAMA(png, pngInfo, &fileGamma);
			if (res > 0)
			{
				hasGamma = true;
				props->gamma = fileGamma;
			}
			
			// is a Chroma supplied ? (cHRM chunk present - cotains x,y for white point and primaries)
			double wpx, wpy, rx, ry, gx, gy, bx, by; // white point and primaries
			res = png_get_cHRM(png, pngInfo, &wpx, &wpy, &rx, &ry, &gx, &gy, &bx, &by);
			if (res > 0)
			{
				hasChroma = true;
				props->whitepoint[0] = wpx;
				props->whitepoint[1] = wpy;
				if (props->chromaticPrimaries.size() != 6) 
					props->chromaticPrimaries.resize(6);
				props->chromaticPrimaries[0] = rx;
				props->chromaticPrimaries[1] = ry;
				props->chromaticPrimaries[2] = gx;
				props->chromaticPrimaries[3] = gy;
				props->chromaticPrimaries[4] = bx;
				props->chromaticPrimaries[5] = by;
			}
			
			// now we need to differentiate between color or gray scale images!
			if ( pm.isRGB() ) 
			{
				//
				// COLOR IMAGE
				//
				// Here's what we do if no ICC profile is supplied:
				// 1) if the sRGB tag is set, we assign an sRGB profile to the image - done
				// 2) if there is a cHRM AND gAMA, create an "on the fly profile" and assign it to the image - done
				// 3) if cHRM but no gAMA assigned, assume Gamma=2.2, create an "on the fly profile" and assign it to the image - done
				// 4) if gAMA (but no cHRM) assigned, assume cHRM = Rec. 709, create an "on the fly profile" and assign it to the image - done
				// 5) if nothing supplied, do not assign anything.
				
				if ( issRgb )
				{
					// 1)
					ColorManager cms;
					cmsHPROFILE profile = cms.createSrgbProfile();
					cms.embeddProfile(image, profile);
					cms.closeProfile(profile);
				}
				else if ( hasGamma | hasChroma )
				{
					// 2), 3), 4)
					ColorManager cms;

					if ( ! hasGamma ) 
					{
						fileGamma = DEFAULT_FILE_GAMMA;	// 3
					}
					
					cmsHPROFILE profile;
					if ( ! hasChroma )
					{
						// 4
						profile = cms.createRgbProfile(&CCIR_709_WHITEPOINT, &CCIR_709_PRIMARIES, 1.0/fileGamma); // not sure if gamma is right or inverted ...
					}
					else
					{
						// 2, 3
						cmsCIExyY whitepoint = { wpx, wpy, 1.0 };
						cmsCIExyYTRIPLE primaries = { { rx, ry, 1.0 }, { gx, gy, 1.0 }, { bx, by, 1.0 } };
						profile = cms.createRgbProfile(&whitepoint, &primaries, 1.0/fileGamma);	// not sure if gamma is right or inverted ...
					}
					cms.embeddProfile(image, profile);
					cms.closeProfile(profile);
				}
				else
				{
					// 5
				}
			}
			else
			{
				//
				// GRAYSCALE IMAGE
				//
				// Here's what we do if no ICC profile is supplied:
				// 1) if the sRGB tag is set, we assign an on-the-fly Gray profile D65 (sRGB white point) and gamma 2.2 (average sRGB gamma) - done
				// 2) if there is a cHRM AND gAMA, we assign an on-the-fly Gray profile with white point from cHRM and gAMA - done
				// 3) if cHRM but no gAMA assigned, assume Gamma=2.2, create an "on the fly profile" Gray profile with white point from cHRM - done
				// 4) if gAMA (but no cHRM) assigned, assume D50 white point and use gAMA - done 
				// 5) if nothing supplied, do not assign anything.
				if ( issRgb )
				{
					// 1)
					ColorManager cms;
					cmsHPROFILE profile = cms.createGrayProfile( &ColorManager::WHITE_POINT_D65, 2.2 );
					cms.embeddProfile(image, profile);
					cms.closeProfile(profile);					
				}
				else if ( hasGamma | hasChroma )
				{
					// 2), 3), 4)
					ColorManager cms;
					cmsCIExyY whitePoint;
					double gamma;

					if ( hasGamma ) 
					{
						gamma = 1/fileGamma;	// 4, 2
					}
					else
					{
						gamma = 2.2;	// 3
					}
					if ( hasChroma )
					{
						whitePoint = { wpx, wpy, 1.0 }; // 2, 3
					}
					else
					{
						whitePoint = ColorManager::WHITE_POINT_D50; // 4
					}
					cmsHPROFILE profile = cms.createGrayProfile( &whitePoint, gamma); // not sure if gamma is right or inverted ...
					cms.embeddProfile(image, profile);
					cms.closeProfile(profile);
				}
				else
				{
					// 5
				}		
			} // end of if (RGB/GRAY)
		} // end of if (!hasICC)

		//////////////////////////////////////////////////////////////////////////
		/// Read the Pixels
		//////////////////////////////////////////////////////////////////////////
		ts.setStamp(5);

		png_start_read_image(png);

		ts.setStamp(6);

#if READ_SINGLE_ROWS
		for (size_t iRow = 0; iRow < height; iRow++)
		{
			png_read_row(png, reinterpret_cast<png_bytep>(image->getLine(iRow)), NULL);
		}
#else
		png_bytepp lines = new png_bytep[height];
		for (size_t iRow = 0; iRow < height; iRow++)
		{
			lines[iRow] = reinterpret_cast<png_bytep>(image->getLine(iRow));
		}
		png_read_image(png, lines);
		delete[] lines;
#endif
		ts.setStamp(7);

		png_read_end(png, endInfo);

		ts.setStamp(8);

		png_destroy_read_struct(&png, &pngInfo, &endInfo);
//		fclose(pFile);
		if ( _data )
		{
			_data.reset();
			_dataCur = nullptr;
			_dataLeft = 0;
		}
		
		onSubImageRead(1);	// we have read image #1
	}
	catch (std::exception e)
	{
		png_destroy_read_struct(&png, &pngInfo, &endInfo);
		if ( _data )
		{
			_data.reset();
			_dataCur = nullptr;
			_dataLeft = 0;
		}
//		fclose(pFile);
		throw;
	}

	std::string times = ts.toString();
	onSubImageRead(0);	// we are done!

}

// colors:
// - if the image has an icc profile, we write that.
void ImageCoderPng::write(const std::string & filename)
{
	throw std::runtime_error("writing PNG files not implemented");
}
void ImageCoderPng::write(std::ostream & stream)
{
	throw std::runtime_error("writing PNG files not implemented");
}
