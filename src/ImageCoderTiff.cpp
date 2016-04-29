/* ImageCoderTiff.cpp
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
#include "ImageCoderTiff.h"

#include <tiffio.h>
#include <tiffio.hxx>
#include <ctime>


ImageCoderTiff::ImageCoderTiff(Image* img) : ImageCoder(IE_TIFF, img)
{
}

ImageCoderTiff::~ImageCoderTiff()
{
}

bool ImageCoderTiff::canEncode()
{
	return true;
}

bool ImageCoderTiff::canDecode()
{
	return true;
}
bool ImageCoderTiff::canEncode( PixelMode pixelMode )
{
	switch (pixelMode)
	{
	case PixelMode::GRAY8:   return true;
	case PixelMode::GRAY16:  return true;
	case PixelMode::AGRAY8:  return true;
	case PixelMode::AGRAY16: return true;
	case PixelMode::RGB8:    return true;
	case PixelMode::RGB16:   return true;
	case PixelMode::ARGB8:   return true;
	case PixelMode::ARGB16:  return true;
	case PixelMode::CMYK8:   return true;
	case PixelMode::CMYK16:  return true;
	case PixelMode::CMYKA8:  return true;
	case PixelMode::CMYKA16: return true;
	}
	return false;
}


// reads baseline tiffs, except "mask only" - plus "SEPARATED" assuming CMYK (4 samples per pixel) or RGB (3 samples per pixel)
void ImageCoderTiff::read(const std::string & filename)
{
	TIFF* tif = TIFFOpen(filename.c_str(), "rM");
	if (tif == nullptr)
		throw std::runtime_error("failed to open TIFF file " + filename);

	readAndClose(tif);
}

void ImageCoderTiff::read(std::istream & stream)
{
	TIFF* tif = TIFFStreamOpen("memory", &stream);
	if (tif == nullptr)
		throw std::runtime_error("failed to open TIFF stream");

	readAndClose(tif);
}

void ImageCoderTiff::write(const std::string & filename)
{
	TIFF* tif = TIFFOpen(filename.c_str(), "w");
	if (tif == nullptr)
		throw std::runtime_error("failed to open TIFF file " + filename + " for writing");

	writeAndClose(tif);
}

void ImageCoderTiff::write(std::ostream & stream)
{
	TIFF* tif = TIFFStreamOpen("memory", &stream);
	if (tif == nullptr)
		throw std::runtime_error("failed to open TIFF stream for writing");

	writeAndClose(tif);
}

void ImageCoderTiff::writeAndClose( TIFF* tif )
{
	uint16_t compression = static_cast<uint16>(props->compression);
	uint16_t planarConfiguartion = props->writeSeparatedPlanes ? PLANARCONFIG_SEPARATE : PLANARCONFIG_CONTIG;
	// todo: allow to write planar files. Not yet implemented.

	try
	{
		if (props->embedOtherInfo)
		{
			if (!TIFFSetField(tif, TIFFTAG_SOFTWARE, MFIMAGELIB_ID) )
				throw std::runtime_error("failed to write tiff");
		}

		if (!TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, image->getWidth()))
			throw std::runtime_error("failed to write tiff");

		if (!TIFFSetField(tif, TIFFTAG_IMAGELENGTH, image->getHeight()))
			throw std::runtime_error("failed to write tiff");

		if (!TIFFSetField(tif, TIFFTAG_RESOLUTIONUNIT, RESUNIT_INCH))
			throw std::runtime_error("failed to write tiff");

		if (!TIFFSetField(tif, TIFFTAG_XRESOLUTION, static_cast<float>(image->getResolutionX())))
			throw std::runtime_error("failed to write tiff");

		if (!TIFFSetField(tif, TIFFTAG_YRESOLUTION, static_cast<float>(image->getResolutionY())))
			throw std::runtime_error("failed to write tiff");

		if (!TIFFSetField(tif, TIFFTAG_FILLORDER, FILLORDER_MSB2LSB))
			throw std::runtime_error("failed to write tiff");

		if (!TIFFSetField(tif, TIFFTAG_COMPRESSION, compression))
			throw std::runtime_error("failed to write tiff");

		if (!TIFFSetField(tif, TIFFTAG_PLANARCONFIG, planarConfiguartion))
			throw std::runtime_error("failed to write tiff");

		uint16_t bpp;
		uint16_t pmi;
		uint16_t spp;
		bool alpha = false;
		bool internalAlphaPlaneReorder = false;

		switch (image->getPixelMode())
		{
			// todo: AGRAY and CMYKA!
		case PixelMode::GRAY8:   bpp = 8;  spp = 1; pmi = PHOTOMETRIC_MINISBLACK; break;
		case PixelMode::GRAY16:  bpp = 16; spp = 1; pmi = PHOTOMETRIC_MINISBLACK; break;
		case PixelMode::AGRAY8:  bpp = 8;  spp = 2; pmi = PHOTOMETRIC_MINISBLACK; internalAlphaPlaneReorder = true; alpha = true;	break;
		case PixelMode::AGRAY16: bpp = 16; spp = 2; pmi = PHOTOMETRIC_MINISBLACK; internalAlphaPlaneReorder = true; alpha = true;	break;
		case PixelMode::RGB8:    bpp = 8;  spp = 3; pmi = PHOTOMETRIC_RGB;        break;
		case PixelMode::RGB16:   bpp = 16; spp = 3; pmi = PHOTOMETRIC_RGB;        break;
		case PixelMode::ARGB8:   bpp = 8;  spp = 4; pmi = PHOTOMETRIC_RGB;        internalAlphaPlaneReorder = 0; alpha = true;	break;
		case PixelMode::ARGB16:  bpp = 16; spp = 4; pmi = PHOTOMETRIC_RGB;        internalAlphaPlaneReorder = 0; alpha = true;	break;
		case PixelMode::CMYK8:   bpp = 8;  spp = 4; pmi = PHOTOMETRIC_SEPARATED;  break;
		case PixelMode::CMYK16:  bpp = 16; spp = 4; pmi = PHOTOMETRIC_SEPARATED;  break;
		case PixelMode::CMYKA8:  bpp = 8;  spp = 5; pmi = PHOTOMETRIC_SEPARATED;  alpha = true;	break;
		case PixelMode::CMYKA16: bpp = 16; spp = 5; pmi = PHOTOMETRIC_SEPARATED;  alpha = true;	break;
		}

		if (!TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, bpp))
			throw std::runtime_error("failed to write tiff");

		if (!TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, pmi))
			throw std::runtime_error("failed to write tiff");

		if (pmi == PHOTOMETRIC_SEPARATED)
		{
			if (!TIFFSetField(tif, TIFFTAG_INKSET, INKSET_CMYK))
				throw std::runtime_error("failed to write tiff");
		}

		if (!TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, spp))
			throw std::runtime_error("failed to write tiff");

				if (alpha)
		{
			uint16_t extraSampleCount = 1;
			uint16_t extraSampleTypes[1] = { EXTRASAMPLE_ASSOCALPHA };
			if (!TIFFSetField(tif, TIFFTAG_EXTRASAMPLES, extraSampleCount, extraSampleTypes))
				throw std::runtime_error("failed to write tiff");
		}
		else if (!TIFFSetField(tif, TIFFTAG_EXTRASAMPLES, 0, nullptr))
			throw std::runtime_error("failed to write tiff");

		if (!TIFFSetField(tif, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT))
			throw std::runtime_error("failed to write tiff");

		if (!getIccProfile().empty())
		{
			if (!TIFFSetField(tif, TIFFTAG_ICCPROFILE, static_cast<uint32_t>(getIccProfile().size()), getIccProfile().c_str()))
				throw std::runtime_error("failed to embedd icc profile to tiff");
		}

		if (props->embedTimestamp)
		{
			std::stringstream timestamp;
			auto now_c = std::time(NULL);
#if defined(__GNUC__) && (__GNUC__ < 5)
			// workarround for std::put_time not implemented before gcc 5
			{
				struct tm* now_loc = std::localtime(&now_c);
				char buffer[128] = { 0 };
				strftime(buffer, 127, "%Y:%m:%d %H:%M:%S", now_loc);
				timestamp << std::string(buffer);
			}
#else
			timestamp << std::put_time(std::localtime(&now_c), "%Y:%m:%d %H:%M:%S");
#endif
			if (!TIFFSetField(tif, TIFFTAG_DATETIME, timestamp.str().c_str()))
				throw std::runtime_error("failed to write tiff");
		}

		// WRITING IMAGE DATA
		// Note that we don't support writing tiles ...
		// Further note that the planar mode is a bit tricky; you need to write all LINES of one PLANE first ... 
		// There's no random access in the way it's written.

		size_t bypp = bpp / 8;
		size_t planarLineLength = image->getWidth() * bypp;
		size_t linebytes = spp * planarLineLength;   // in planar config, each line one after the other ...
		std::unique_ptr<char[]> lineBuffer(new char[linebytes]);	// needed for pixel organizations that we can't directly write to the file... Alpha and planar. ANd actually, for planar, the size is too big.

		// We set the strip size of the file to be size of one row of pixels ... 
		uint32_t rowsPerStrip = TIFFDefaultStripSize(tif, 0);
		if (!TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP, rowsPerStrip))
			throw std::runtime_error("failed to write tiff");

		// stuff below is actually like for planar ...
		size_t numPlanes = props->writeSeparatedPlanes ? spp : 1;
		std::unique_ptr<size_t[]> internalPlaneIndex(new size_t[numPlanes]);
		if (props->writeSeparatedPlanes)
		{
			for (size_t iPlaneExternal = 0; iPlaneExternal < numPlanes; iPlaneExternal++)
				internalPlaneIndex[iPlaneExternal] = iPlaneExternal + (internalAlphaPlaneReorder ? 1 : 0); // when alpha is present in AGRAY, ARGB: 0->1, 1->2... n=0
			if (internalAlphaPlaneReorder)
				internalPlaneIndex[numPlanes - 1] = 0; // this is the n=0 mentioned above...
		}
		size_t planePixelOffset = bypp*(spp - 1);

		// we loop through each plane --- of which there's only one if we write contingous

		for (uint16_t iPlane = 0; iPlane < numPlanes; iPlane++)
		{
			for (uint32_t iLine = 0; iLine < image->getHeight(); iLine++)
			{
				uint8_t* line = image->getLine(iLine);

				if (props->writeSeparatedPlanes)
				{
					uint8_t* p = reinterpret_cast<uint8_t*>(lineBuffer.get());
					uint8_t* q = reinterpret_cast<uint8_t*>(line);
					q += bypp*internalPlaneIndex[iPlane]; // skip to the first sample of the internal plane we are looking for and go from there.
					for (size_t iCol = 0; iCol < image->getWidth(); iCol++)
					{
						*p++ = *q++;
						if (bypp == 2)
							*p++ = *q++;	// much faster than a for-loop over bytes - but needs adaption if we ever write larger than 16 bit/pixel (e.g. float)
						q += planePixelOffset; // jump to next pixel in same plane
					}
					TIFFWriteScanline(tif, lineBuffer.get(), iLine, iPlane);
				}
				else
				{
					switch (image->getPixelMode())
					{
					case PixelMode::GRAY8:
					case PixelMode::GRAY16:
					case PixelMode::RGB8:
					case PixelMode::RGB16:
					case PixelMode::CMYK8:
					case PixelMode::CMYK16:
					case PixelMode::CMYKA8:
					case PixelMode::CMYKA16:
						TIFFWriteScanline(tif, line, iLine, 0);
						break;

					case PixelMode::ARGB8: // todo: planar from here down 
					{
											   uint8_t* p = reinterpret_cast<uint8_t*>(lineBuffer.get());
											   uint8_t* q = reinterpret_cast<uint8_t*>(line);
											   for (size_t iCol = 0; iCol < image->getWidth(); iCol++)
											   {
												   *p++ = q[1];	// R
												   *p++ = q[2];	// G
												   *p++ = q[3];	// B
												   *p++ = q[0];	// A
												   q += 4;
											   }
											   TIFFWriteScanline(tif, lineBuffer.get(), iLine, 0);
											   break;
					}
					case PixelMode::ARGB16:
					{
											  uint16_t* p = reinterpret_cast<uint16_t*>(lineBuffer.get());
											  uint16_t* q = reinterpret_cast<uint16_t*>(line);
											  for (size_t iCol = 0; iCol < image->getWidth(); iCol++)
											  {
												  *p++ = q[1];	// R
												  *p++ = q[2];	// G
												  *p++ = q[3];	// B
												  *p++ = q[0];	// A
												  q += 4;
											  }
											  TIFFWriteScanline(tif, lineBuffer.get(), iLine, 0);
											  break;
					}
					case PixelMode::AGRAY8:
					{
											  uint8_t* p = reinterpret_cast<uint8_t*>(lineBuffer.get());
											  uint8_t* q = reinterpret_cast<uint8_t*>(line);
											  for (size_t iCol = 0; iCol < image->getWidth(); iCol++)
											  {
												  *p++ = q[1];	// G
												  *p++ = q[0];	// A
												  q += 2;
											  }
											  TIFFWriteScanline(tif, lineBuffer.get(), iLine, 0);
											  break;

					}
					case PixelMode::AGRAY16:
					{
											   uint16_t* p = reinterpret_cast<uint16_t*>(lineBuffer.get());
											   uint16_t* q = reinterpret_cast<uint16_t*>(line);
											   for (size_t iCol = 0; iCol < image->getWidth(); iCol++)
											   {
												   *p++ = q[1];	// G
												   *p++ = q[0];	// A
												   q += 2;
											   }
											   TIFFWriteScanline(tif, lineBuffer.get(), iLine, 0);
											   break;
					}
					} // end of switch(pixelMode)
				} // end of contingous mode
			} // end of for-line-loop
		} // end of for-plane-loop
		TIFFClose(tif);
	}
	catch (std::runtime_error)
	{
		TIFFClose(tif);
		throw;
	}
}

void ImageCoderTiff::readAndClose(TIFF* tif)
{
	try
	{
		int imageCount = 0;
		bool continueToRead;
		do { /* loop over multiple images in files */
			imageCount++;

			///////////////////////////////////////////////
			// size and resolution
			///////////////////////////////////////////////
			uint32_t height;
			if (!TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &height))
				throw std::runtime_error("missing ImageLength TAG in TIFF");
			uint32_t width;
			if (!TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &width))
				throw std::runtime_error("missing ImageWidth TAG in TIFF");
			double dpiX = 72.0; // default
			double dpiY = 72.0; // default
			{
				uint16_t resolutionUnit = RESUNIT_INCH; // default
				TIFFGetField(tif, TIFFTAG_RESOLUTIONUNIT, &resolutionUnit);

				float res;
				switch (resolutionUnit) {
				case RESUNIT_INCH:
					if (TIFFGetField(tif, TIFFTAG_XRESOLUTION, &res))
						dpiX = res;
					if (TIFFGetField(tif, TIFFTAG_YRESOLUTION, &res))
						dpiY = res;
					else
						dpiY = dpiX;	// if no vertical resolution supplied, we use horizontal as vertical
					break;
				case RESUNIT_CENTIMETER:
					if (TIFFGetField(tif, TIFFTAG_XRESOLUTION, &res))
						dpiX = res * 2.54;
					if (TIFFGetField(tif, TIFFTAG_YRESOLUTION, &res))
						dpiY = res * 2.54;
					else
						dpiY = dpiX; // if no vertical resolution supplied, we use horizontal as vertical
					break;
				}
			}
			
			////////////////////////////////////////
			// orientation and fillorder
			// - we only support TOPLEFT orientation
			// - we only support MSB2LSB fillorder
			////////////////////////////////////////
			{
				uint16_t orientation;
				if (TIFFGetField(tif, TIFFTAG_ORIENTATION, &orientation) && orientation != ORIENTATION_TOPLEFT)
					throw std::runtime_error("mfimage lib does not support TIFFTAG_ORIENTATION=" + std::to_string(orientation));

			}
			uint16_t fillOrder = FILLORDER_MSB2LSB; // default fillorder
			TIFFGetField(tif, TIFFTAG_FILLORDER, &fillOrder);
			
			/////////////////////////
			// Planar configuration
			/////////////////////////
			uint16_t planarConfig = PLANARCONFIG_CONTIG;
			TIFFGetField(tif, TIFFTAG_PLANARCONFIG, &planarConfig);
			bool isSeparated = planarConfig == PLANARCONFIG_SEPARATE;
			
			/////////////////////////
			// Halftone-hints
			// Spec: 2 shorts ... "Conveys to the halftone function the range of gray levels within a colorimetrically-specified image that should retain tonal detail."
			/////////////////////////
			uint16_t halftoneHints_White = 0; // default
			uint16_t halftoneHints_Black = 0; // default
			if ( props->applyHalftoneHintsOnRead )
				TIFFGetField( tif, TIFFTAG_HALFTONEHINTS, &halftoneHints_White, &halftoneHints_Black );


			/////////////////////////
			// Samples per Pixel (channels), Bits per Sample (bpc)
			/////////////////////////
			uint16_t samplesPerPixel = 1;
			TIFFGetField(tif, TIFFTAG_SAMPLESPERPIXEL, &samplesPerPixel);

			uint16_t bitsPerSample; // libtiff doesn't support different bits per sample for different samples (planes), which tiff spec would support !
			TIFFGetField(tif, TIFFTAG_BITSPERSAMPLE, &bitsPerSample);


			/////////////////////////
			// extrasamples - we boild this down to "extraAlphaSample" and ignore all the rest
			// - extraAlphaSample == -1 -> no alpha channel
			// - extraAlphaSample >= 0 -> extra channel index is alpha
			/////////////////////////
			int extraAlphaSample = -1;
			uint16_t extraSampleCount = 0;
			{
//				uint16_t extraSampleCount = 0;
				uint16_t *extraSampleTypes;
				if ( TIFFGetField(tif, TIFFTAG_EXTRASAMPLES, &extraSampleCount, &extraSampleTypes) && extraSampleCount && extraSampleTypes )
				{
					for (int i = 0; i < extraSampleCount; i++) 
					{
						if ( extraSampleTypes[i] == EXTRASAMPLE_ASSOCALPHA )
						{
							extraAlphaSample = i; break;
						}
					}
				}
			}

			/////////////////////////////////////
			// Is the image written in tiles ?
			/////////////////////////////////////
			bool tiled = true;
			uint32_t tileWidth, tileLength;
			if ( 0 == TIFFGetField( tif, TIFFTAG_TILEWIDTH, &tileWidth ) )
				tiled = false;
			if ( 0 == TIFFGetField( tif, TIFFTAG_TILELENGTH, &tileLength ) )
				tiled = false;
			// if it is tiled, create a buffer to read the complete image first.
			// this is the "easiest" way, also it's counter the idea of tiles:
			// to read a partial image!
			std::unique_ptr<uint8_t[]> tiledImageBuffer;
			_tiles_planeSize = 0;
			_tiles_numPlanes = 0;
			if ( tiled ) 
			{
				tmsize_t   totalTileSize = TIFFTileSize(tif);
				tdata_t  bufSingleTile = _TIFFmalloc(totalTileSize);
				tmsize_t scanlinesize  = TIFFScanlineSize(tif);
				tmsize_t tilelinesize  = TIFFTileRowSize(tif);
				// note that for a planar configuration, each tile contains one PLANE only... So our total buffer needs to increaase for planar config
				_tiles_numPlanes = 1;
				_tiles_planeSize = static_cast<uint32_t>(height*scanlinesize);
				if ( isSeparated )
				{
//					_tiles_numPlanes = samplesPerPixel + (( extraAlphaSample!=-1 ) ? 1 : 0 );
					_tiles_numPlanes = samplesPerPixel + extraSampleCount;
				}
				tiledImageBuffer.reset( new uint8_t[_tiles_planeSize*_tiles_numPlanes] );
				
				for (uint16_t iPlane = 0; iPlane<_tiles_numPlanes; iPlane++)
				{
					// planeIndex is the index of the plane in the TIFF file.
					// our internal index is iPlane.
					uint16_t planeIndex = iPlane; //(iPlane>=samplesPerPixel) ? extraAlphaSample : iPlane; 
					size_t internalPlaneOffset = _tiles_planeSize*iPlane;
					
					for (uint32_t y0 = 0; y0 < height; y0 += tileLength)
					{
						size_t tileLineStartByte = 0;
						for (uint32_t x0 = 0; x0 < width; x0 += tileWidth)
						{
							TIFFReadTile( tif, bufSingleTile, x0, y0, 0, planeIndex); // read a complete tile to bufSingleTile (or tiles plane in planar config)
							uint8_t* bufLine = reinterpret_cast<uint8_t*>(bufSingleTile);					
							size_t thisTileLineSize = std::min( (unsigned long)(scanlinesize-tileLineStartByte), (unsigned long)tilelinesize );
							for ( uint32_t y=y0; y<std::min(height,y0+tileLength); y++ )
							{
								std::memcpy( tiledImageBuffer.get()+internalPlaneOffset+y*scanlinesize+tileLineStartByte, bufLine, thisTileLineSize );
								bufLine += tilelinesize;
							}
							tileLineStartByte += static_cast<size_t>(tilelinesize);
						}
					}
				}
				_TIFFfree( bufSingleTile );
			}
			
			////////////////////////////////
			// 
			// Photometrix interpretation of the image
			//
			////////////////////////////////
			uint16_t photometricInterpretation;
			if (!TIFFGetField(tif, TIFFTAG_PHOTOMETRIC, &photometricInterpretation))
				throw std::runtime_error("missing Photometric TAG in TIFF");

			uint16_t indexed = 0;
			bool inverted = false;
			switch (photometricInterpretation)
			{
			case PHOTOMETRIC_MINISWHITE:	/* min value is white */
				inverted = true;
				
			case PHOTOMETRIC_MINISBLACK:	/* min value is black */
				if ( halftoneHints_Black > halftoneHints_White )
					std::swap( halftoneHints_Black, halftoneHints_White );

				if (bitsPerSample != 1 && bitsPerSample != 4 && bitsPerSample != 8 && bitsPerSample != 16)
					throw std::runtime_error("unsupported BitsPerSample for grayscale image: " + std::to_string(bitsPerSample));

				readBilevelOrGray(tif, width, height, dpiX, dpiY, samplesPerPixel, bitsPerSample, extraAlphaSample, inverted, isSeparated, halftoneHints_White, halftoneHints_Black, tiledImageBuffer.get() );
				break;
				
			case PHOTOMETRIC_RGB:	/* RGB color model */
				TIFFGetField(tif, TIFFTAG_INDEXED, &indexed); // note that we only support indexed RGB
				if (indexed == 0)
				{
					if (bitsPerSample != 8 && bitsPerSample != 16)
						throw std::runtime_error("unsupported BitsPerSample for RGB image: " + std::to_string(bitsPerSample));
					readRGB(tif, width, height, dpiX, dpiY, samplesPerPixel, bitsPerSample, extraAlphaSample, isSeparated, tiledImageBuffer.get() );
					break;
				}
				else
				{
					// do not break !!!! this has the same meaning as "PALETTE".
				}
				
			case PHOTOMETRIC_PALETTE:  /* color map indexed */
				if (bitsPerSample != 4 && bitsPerSample != 8)
					throw std::runtime_error("unsupported BitsPerSample for PALETTE image: " + std::to_string(bitsPerSample));
				readRGBPalette(tif, width, height, dpiX, dpiY, samplesPerPixel, bitsPerSample, extraAlphaSample, isSeparated, tiledImageBuffer.get() );
				break;
				
			case PHOTOMETRIC_SEPARATED:	/* !color separations  -- - typically cmyk */
				{
					uint16_t inkset = INKSET_CMYK;
					TIFFGetField(tif, TIFFTAG_INKSET, &inkset);
					if (inkset != INKSET_CMYK)
						throw std::runtime_error("unsupported Multiink-Separations. mfimage library only support CMYK separations.");
					if (samplesPerPixel < 4)
						throw std::runtime_error("bogus samples per Pixel for CMYK separations.");
					if (bitsPerSample != 8 && bitsPerSample != 16)
						throw std::runtime_error("unsupported BitsPerSample for CMYK separations: " + std::to_string(bitsPerSample));
				}
				readCMYK(tif, width, height, dpiX, dpiY, samplesPerPixel, bitsPerSample, extraAlphaSample, isSeparated, tiledImageBuffer.get() );
				break;
				
			case PHOTOMETRIC_YCBCR:
				throw std::runtime_error("unsupported Photometric TAG (color encoding): PHOTOMETRIC_YCBCR");
			case PHOTOMETRIC_CIELAB:	/* !1976 CIE L*a*b* */
				throw std::runtime_error("unsupported Photometric TAG (color encoding): PHOTOMETRIC_CIELAB");
				// Meaning: 8 bit: L [0..255], a [-128 - 127], b [-128 - 127]
				// 16 bit: L [0..65535], a, b [-32768 - 32767]
				// also see ICCLAB
			case PHOTOMETRIC_ICCLAB:	/* ICC L*a*b* [Adobe TIFF Technote 4] */
				throw std::runtime_error("unsupported Photometric TAG (color encoding): PHOTOMETRIC_ICCLAB (Adobe TIFF Technote 4)");
				// Meaning: 8 bit: L [0..255], a, b [0 - 255]
				// !!!! 16 bit: L [0..65280], a, b [0 - 65535]
				// allowed bps = 8, 16				
				// allowed number of channels: 1 (L only), 3 (Lab)
				// Whitepoint: Default D50, might be other!
				
			case PHOTOMETRIC_ITULAB:	/* ITU L*a*b* */
				throw std::runtime_error("unsupported Photometric TAG (color encoding): PHOTOMETRIC_ITULAB");
			case PHOTOMETRIC_MASK:		/* $holdout mask */
				throw std::runtime_error("unsupported Photometric TAG (color encoding): PHOTOMETRIC_MASK");
			case PHOTOMETRIC_CFA:
				throw std::runtime_error("unsupported Photometric TAG (color encoding): PHOTOMETRIC_CFA");
			case PHOTOMETRIC_LOGL:		/* CIE Log2(L) */
				throw std::runtime_error("unsupported Photometric TAG (color encoding): PHOTOMETRIC_LOGL");
			case PHOTOMETRIC_LOGLUV:	/* CIE Log2(L) (u',v') */
				throw std::runtime_error("unsupported Photometric TAG (color encoding): PHOTOMETRIC_LOGLUV");
			default:
				throw std::runtime_error("unsupported Photometric TAG (color encoding): " + std::to_string(photometricInterpretation));
				break;
				
			}

			// read ICC Profile if available
			uint32_t iccLen;
			void* iccBuffer;
			if (TIFFGetField(tif, TIFFTAG_ICCPROFILE, &iccLen, &iccBuffer))
				getIccProfile().assign(reinterpret_cast<char*>(iccBuffer), iccLen);
			else
				getIccProfile().clear();
				

			continueToRead = onSubImageRead(imageCount);	// we have read image #imageCount
        } while (continueToRead && TIFFReadDirectory(tif));

		onSubImageRead(0);	// we are done!
		TIFFClose(tif);		
	}
	catch (std::runtime_error)
	{
		TIFFClose(tif);
		throw;
	}
}

// same as TiffReadScanLine, but reads line either from tiled image buffer or actually calls TiffReadScanLine
int ImageCoderTiff::readscanline(TIFF* tif, uint8_t* tiledImageBuffer, uint32_t scanlinesize, void* dest, uint32_t row, uint16_t sample/*=0*/)
{
	if ( tiledImageBuffer )
	{
		std::memcpy( dest, tiledImageBuffer + _tiles_planeSize*sample + scanlinesize*row, scanlinesize );
		return 0;
	}
	else
	{
		return TIFFReadScanline(tif, dest, row, sample ); // same for separated/contig
	}	
}

void ImageCoderTiff::readBilevelOrGray(TIFF* tif, uint32_t width, uint32_t height, double dpiX, double dpiY, uint16_t samplesPerPixel, uint16_t bitsPerSample, int extraAlphaSample, bool inverted, bool separated, uint16_t highlight, uint16_t shadow, uint8_t* tiledImageBuffer )
{
	bool hasAlpha = samplesPerPixel > 1 && extraAlphaSample != -1;

	// with ther functions we are using, libtiff is already internally auto-corrcting the fillorder with TIFFReverseBits. We don't have to care about it. So we are always assuming MSB2LSB
	static const unsigned char oneBitMask[8] = { 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01 };
	tdata_t buf;
	tmsize_t scanlinesize = TIFFScanlineSize(tif);
	buf = _TIFFmalloc(scanlinesize);
	//////////////////////////////
	// halftoneHints --- basically an "input levels" filter!
	///////////////////////////////
	bool halftoneHints = (highlight || shadow);
	uint16_t dynRange;
	if ( halftoneHints && bitsPerSample == 4 )
	{
		// make sure they are correct for 4bpc (they don't make sense for 1bpc)
		highlight <<= 4;
		shadow <<= 4;
	}
	if ( halftoneHints )
	{
		uint16_t maxValue = (1<<samplesPerPixel)-1;
		// now me modify shadow to be able to calc better ...
		if ( shadow > 0 ) shadow--;
		if ( highlight < maxValue ) highlight++;
		// just a helper to calc faster ...
		dynRange = highlight - shadow; 
		// if halftoneHints are min and max, then don't do anything!
		if ( shadow == 0 && highlight == maxValue )
			halftoneHints = false; // it's normalized already, makes no sense to calc anything
	}

	if ( hasAlpha )
	{
		if (bitsPerSample <= 8)
			image->create(width, height, PixelMode::AGRAY8, dpiX, dpiY);
		else
			image->create(width, height, PixelMode::AGRAY16, dpiX, dpiY);
	}
	else
	{
		if (bitsPerSample <= 8)
			image->create(width, height, PixelMode::GRAY8, dpiX, dpiY);
		else
			image->create(width, height, PixelMode::GRAY16, dpiX, dpiY);
	}


	for (uint32_t row = 0; row < height; row++)
	{
		if ( samplesPerPixel == 1 && ( bitsPerSample == 8 || bitsPerSample == 16 ) && (! inverted) && (! halftoneHints) )
		{
			readscanline( tif, tiledImageBuffer, static_cast<uint32_t>(scanlinesize), image->getLine(row), row );
		}
		else if ( separated )
		{
			throw std::runtime_error( "Separated not implemented for gray levels" );	
		}
		else
		{
			// todo: tiles
			uint8_t* data = image->getLine(row);
			uint16_t* data16 = reinterpret_cast<uint16_t*>(data);
			readscanline(tif, tiledImageBuffer, static_cast<uint32_t>(scanlinesize), buf, row);
			uint8_t val8;
			uint16_t val16;
			int offset = 0;
			int alphaOffset;
			for (uint32_t col = 0; col < width; col++)
			{
				if ( hasAlpha ) 
					alphaOffset = offset+1+extraAlphaSample;
				switch (bitsPerSample)
				{
				case 16: // 2^16 tones of gray
					val16 = reinterpret_cast<uint16_t*>(buf)[offset];
					val16 = inverted ? ~val16 : val16;
					if ( halftoneHints )
						val16 = (val16 <= shadow) ? 0 : (val16 >= highlight) ? 65536 : ((val16-shadow)*65536/dynRange);
					if ( hasAlpha )
						*data16++ = reinterpret_cast<uint16_t*>(buf)[alphaOffset];
					*data16++ = val16;
					break;
				case 8: // 256 tones of gray
					val8 = reinterpret_cast<uint8_t*>(buf)[offset];
					if ( halftoneHints )
						val8 = (val8 <= shadow) ? 0 : (val8 >= highlight) ? 255 : ((val8-shadow)*255/dynRange);
					if ( hasAlpha )
						*data++ = reinterpret_cast<uint8_t*>(buf)[alphaOffset];
					*data++ = inverted ? ~val8 : val8;
					break;
				case 4: // 16 tones of gray
					if ( offset & 0x01 )
						val8 = (reinterpret_cast<uint8_t*>(buf)[offset >> 1] & 0xf) << 4;
					else 
						val8 = (reinterpret_cast<uint8_t*>(buf)[offset >> 1] & 0xf0);
					if ( halftoneHints )
						val8 = (val8 <= shadow) ? 0 : (val8 >= highlight) ? 255 : ((val8-shadow)*255/dynRange);
					if ( hasAlpha )
					{
						if ( alphaOffset & 0x01 )
							*data++ = (reinterpret_cast<uint8_t*>(buf)[alphaOffset >> 1] & 0xf) << 4;
						else 
							*data++ = (reinterpret_cast<uint8_t*>(buf)[alphaOffset >> 1] & 0xf0);
					}
					*data++ = inverted ? ~val8 : val8;
					break;
				case 1: // bilevel
					val8 = (reinterpret_cast<uint8_t*>(buf)[offset >> 3] & oneBitMask[offset & 0x7]) ? 255 : 0;
					// halftonehints make no sense for 1 bit.
					if ( hasAlpha )
						*data++ = (reinterpret_cast<uint8_t*>(buf)[alphaOffset >> 3] & oneBitMask[alphaOffset & 0x7]) ? 255 : 0;
					*data++ = inverted ? ~val8 : val8;
					break;
				}
				offset += samplesPerPixel;		
			}
		} // end if samplesperpixel==1 ... / separated / contig
	}
	_TIFFfree(buf);
}


// reads an RGB encoded image as 8 or 16 bit RGB or ARGB image.
// TODO:
// - float interpretation ?
// - other than 8/16 bps ?
// TEST to do:
// - Contigous 16 bps RGB + EXTRASAMPLE_ASSOCALPHA
// - Separated  8 bps RGB + EXTRASAMPLE_ASSOCALPHA
// - Separated 16 bps RGB + EXTRASAMPLE_ASSOCALPHA
// tested:
// - Contigous  8 bps RGB w/o alpha
// - Contigous  8 bps RGB + EXTRASAMPLE_ASSOCALPHA
// - Contigous 16 bps RGB w/o alpha
// - Separated  8 bps RGB w/o alpha
// - Separated 16 bps RGB w/o alpha

void ImageCoderTiff::readRGB(TIFF* tif, uint32_t width, uint32_t height, double dpiX, double dpiY, uint16_t samplesPerPixel, uint16_t bitsPerSample, int extraAlphaSample, bool separated, uint8_t* tiledImageBuffer )
{
	bool hasAlpha = (samplesPerPixel > 3) && (extraAlphaSample != -1);

	tdata_t buf;
	tmsize_t scanlinesize = TIFFScanlineSize(tif);
	buf = _TIFFmalloc(scanlinesize);
	
	if (bitsPerSample == 8 )
		image->create(width, height, hasAlpha ? PixelMode::ARGB8 : PixelMode::RGB8, dpiX, dpiY);
	else 
		image->create(width, height, hasAlpha ? PixelMode::ARGB16 : PixelMode::RGB16, dpiX, dpiY);		
	
	for (uint32_t row = 0; row < height; row++)
	{
		if ( separated )
		{
			int planeIndizesInTiff[4]   = { extraAlphaSample+3, 0, 1, 2 };
			int samplesPerPixelInternal = hasAlpha ? 4 : 3;	// if we have an alpha, we have 4 samples (ARGB) per pixel in our INTERNAL otherwise 3 (RGB)
			int rgbOffset               = hasAlpha ? 0 : 1; // if we have NO alpha, we offset by 1 byte in position calculation...
			for ( int plane=(4-samplesPerPixelInternal); plane<4; plane++ ) // skip first plane index if there's no alpha
			{
				uint8_t* lineData = image->getLine(row); // start of internal data row.
//				TIFFReadScanline(tif, buf, row, planeIndizesInTiff[plane]); // read to internal buffer
				readscanline(tif, tiledImageBuffer, static_cast<uint32_t>(scanlinesize), buf, row, planeIndizesInTiff[plane]);
				for (uint32_t col = 0; col < width; col++)
				{
					int planeOffset = (samplesPerPixelInternal * col) + (plane - rgbOffset);
					if (bitsPerSample == 8)
						lineData[planeOffset] = reinterpret_cast<uint8_t*>(buf)[col];
					else 
						reinterpret_cast<uint16_t*>(lineData)[planeOffset] = reinterpret_cast<uint16_t*>(buf)[col];
				}		
			}
		}	
		else	// contig.
		{
			if ( samplesPerPixel==3 ) // RGB8 or RGB16
//				TIFFReadScanline(tif, getLine(row), row);
				readscanline(tif, tiledImageBuffer, static_cast<uint32_t>(scanlinesize), image->getLine(row), row);
			else
			{
				int samplesPerPixelInternal = hasAlpha ? 4 : 3;	// if we have an alpha, we have 4 samples (ARGB) per pixel in our INTERNAL otherwise 3 (RGB)
				uint8_t* data = image->getLine(row);
//				TIFFReadScanline(tif, buf, row);
				readscanline(tif, tiledImageBuffer, static_cast<uint32_t>(scanlinesize), buf, row);
				for (uint32_t col = 0; col < width; col++)
				{
					if (bitsPerSample == 16)
					{
						uint16_t* p = reinterpret_cast<uint16_t*>(data)+(samplesPerPixelInternal * col);
						if (hasAlpha)
							*p++ = reinterpret_cast<uint16_t*>(buf)[samplesPerPixel*col + (3+extraAlphaSample)];
						*p++ = reinterpret_cast<uint16_t*>(buf)[samplesPerPixel*col + 0];
						*p++ = reinterpret_cast<uint16_t*>(buf)[samplesPerPixel*col + 1];
						*p   = reinterpret_cast<uint16_t*>(buf)[samplesPerPixel*col + 2];
					}
					else
					{
						uint8_t* p = data+(samplesPerPixelInternal * col);
						if (hasAlpha)
							*p++ = reinterpret_cast<uint8_t*>(buf)[samplesPerPixel*col + (3+extraAlphaSample)];
						*p++ = reinterpret_cast<uint8_t*>(buf)[samplesPerPixel*col + 0];
						*p++ = reinterpret_cast<uint8_t*>(buf)[samplesPerPixel*col + 1];
						*p   = reinterpret_cast<uint8_t*>(buf)[samplesPerPixel*col + 2];
					}
				}
			}
		} // end if sep / cont
	} // end for each row
	_TIFFfree(buf);
}

// reads an CMYK encoded image as 8 or 16 bit CMYK image.
// TODO:
// - float interpretation ?
// TESTS:
// - Contigous  8 bps CMYK w/o alpha
// - Contigous 16 bps CMYK w/o alpha
// - Contigous  8 bps CMYK + EXTRASAMPLE_ASSOCALPHA
// - Contigous 16 bps CMYK + EXTRASAMPLE_ASSOCALPHA
// - Separated  8 bps CMYK w/o alpha
// - Separated 16 bps CMYK w/o alpha
// - Separated  8 bps CMYK + EXTRASAMPLE_ASSOCALPHA
// - Separated 16 bps CMYK + EXTRASAMPLE_ASSOCALPHA
void ImageCoderTiff::readCMYK(TIFF* tif, uint32_t width, uint32_t height, double dpiX, double dpiY, uint16_t samplesPerPixel, uint16_t bitsPerSample, int extraAlphaSample, bool separated, uint8_t* tiledImageBuffer)
{
	bool hasAlpha = (samplesPerPixel > 4) && (extraAlphaSample != -1);

	tdata_t buf;
	tmsize_t scanlinesize = TIFFScanlineSize(tif);
	buf = _TIFFmalloc(scanlinesize);

	if (bitsPerSample <= 8)
		image->create(width, height, hasAlpha ? PixelMode::CMYKA8 : PixelMode::CMYK8, dpiX, dpiY);	// 8 bpc
	else
		image->create(width, height, hasAlpha ? PixelMode::CMYKA16 : PixelMode::CMYK16, dpiX, dpiY); // 16 bpc

	int samplesPerPixelInternal = (hasAlpha) ? 5 : 4;	// internal storage

	// read		
	for (uint32_t row = 0; row < height; row++)
	{
		if ( separated )
		{
			// we already have alpha support here ...
			int planeIndizesInTiff[5]   = { 0, 1, 2, 3, extraAlphaSample+4 };
			int numPlanes               = (hasAlpha) ? 5 : 4;
			for ( int plane=0; plane<numPlanes; plane++ )
			{
				uint8_t* data = image->getLine(row);
//				TIFFReadScanline(tif, buf, row, planeIndizesInTiff[plane]);
				readscanline(tif, tiledImageBuffer, static_cast<uint32_t>(scanlinesize), buf, row, planeIndizesInTiff[plane]); // untested for tiles!!!! PROBABLY WRONG
				for (uint32_t col = 0; col < width; col++)
				{
					int planeOffset = (samplesPerPixelInternal * col) + plane;
					if (bitsPerSample == 16)
					{
						reinterpret_cast<uint16_t*>(data)[planeOffset] = reinterpret_cast<uint16_t*>(buf)[col];
					}
					else
					{
						data[planeOffset] = reinterpret_cast<uint8_t*>(buf)[col];
					}
				}		
			}
		}
		else	// contig.
		{
			if ( samplesPerPixel == 4 || (samplesPerPixel==5 && hasAlpha && extraAlphaSample==0) )
//				TIFFReadScanline(tif, getLine(row), row);
				readscanline(tif, tiledImageBuffer, static_cast<uint32_t>(scanlinesize), image->getLine(row), row); // untested for tiles!!!! PROBABLY WRONG
			else
			{
				void * data = image->getLine(row);
//				TIFFReadScanline(tif, buf, row);
				readscanline(tif, tiledImageBuffer, static_cast<uint32_t>(scanlinesize), buf, row); // untested for tiles!!!! PROBABLY WRONG
				for (uint32_t col = 0; col < width; col++)
				{
					// todo: alpha support for samples per Pixel != 5...
					if (bitsPerSample == 16)
					{
						memcpy(reinterpret_cast<uint16_t*>(data)+(samplesPerPixelInternal * col), reinterpret_cast<uint16_t*>(buf)+(samplesPerPixel * col), (hasAlpha && extraAlphaSample==0) ? 10 : 8 );
						if ( hasAlpha && extraAlphaSample > 0 )
							reinterpret_cast<uint16_t*>(data)[samplesPerPixelInternal * col + 4] = reinterpret_cast<uint16_t*>(buf)[samplesPerPixel * col + 4 + extraAlphaSample];
					}
					else
					{
						memcpy(reinterpret_cast<uint8_t*>(data)+(samplesPerPixelInternal * col), reinterpret_cast<uint8_t*>(buf)+(samplesPerPixel * col), (hasAlpha && extraAlphaSample==0) ? 5 : 4 );
						if ( hasAlpha && extraAlphaSample > 0 )
							reinterpret_cast<uint8_t*>(data)[samplesPerPixelInternal * col + 4] = reinterpret_cast<uint8_t*>(buf)[samplesPerPixel * col + 4 + extraAlphaSample];
					}
				}
			}
		}
	}
	_TIFFfree(buf);
}

// reads an RGB palette encoded image as 8 or 16 bit RGB image.
// TODO:
// alpha channel support ? Add tests if. I don't know if there could be an alpha channel for indexed images. It is NOT in the color table.
// TESTS:
// - palette with 8 bit deep RGB info, 256 colors
// - palette with 8 bit deep RGB info, 16 colors
// - palette with 16 bit deep RGB info, 256 colors
// - palette with 16 bit deep RGB info, 16 colors
void ImageCoderTiff::readRGBPalette(TIFF* tif, uint32_t width, uint32_t height, double dpiX, double dpiY, uint16_t samplesPerPixel, uint16_t bitsPerSample, int extraAlphaSample, bool separated, uint8_t* tiledImageBuffer)
{
	if ( separated )
	{
		throw std::runtime_error("No support for planarconfig = separated in Palette/indexed images!");
	}
	
	size_t numColors = (1 << bitsPerSample);
	uint16_t *maps[3] = { 0 };
	TIFFGetField(tif, TIFFTAG_COLORMAP, maps+0, maps+1, maps+2);
	std::vector<Image::Pixel_RGB_16> colorMap;
	colorMap.resize(numColors);
	bool is16bit = false; // any real 16 bit values included in table (bits in the lower byte)?
	
	// each entry in the color map is 16 bits. If they don't have that depth, we reduce them to 8
	for (size_t iCol = 0; iCol < numColors; iCol++)
	{
		// color table in tiff is 16 bit.
		colorMap[iCol].r = maps[0][iCol];
		colorMap[iCol].g = maps[1][iCol];
		colorMap[iCol].b = maps[2][iCol];
		// check if there is really 16 bit deep info in the table, otherwise we go for 8 bit
		if ( (!is16bit) && ( (maps[0][iCol] & 0xff) || (maps[1][iCol] & 0xff) || (maps[2][iCol] & 0xff) ) )
		{
			// there is 16 bit information
			is16bit = true;
		}
	}

	std::vector<Image::Pixel_RGB_8> colorMap_8bpc;
	if ( ! is16bit )
	{
		// create an 8 bit per component color map...
		colorMap_8bpc.resize(numColors);
		// color table in tiff is 16 bit.
		for (size_t iCol = 0; iCol < numColors; iCol++)
		{
			colorMap_8bpc[iCol].r = colorMap[iCol].r >> 8;
			colorMap_8bpc[iCol].g = colorMap[iCol].g >> 8;
			colorMap_8bpc[iCol].b = colorMap[iCol].b >> 8;
		}
	}

	image->create(width, height, is16bit ? PixelMode::RGB16 : PixelMode::RGB8, dpiX, dpiY);
	
	tdata_t buf;
	tmsize_t scanlinesize = TIFFScanlineSize(tif);
	buf = _TIFFmalloc(scanlinesize);
	// read		
	for (uint32_t row = 0; row < height; row++)
	{
//		TIFFReadScanline(tif, buf, row);
		readscanline(tif, tiledImageBuffer, static_cast<uint32_t>(scanlinesize), buf, row); // untested for tiles!!!! PROBABLY WRONG
		void* dataLine = image->getLine(row);
		uint8_t *in = reinterpret_cast<uint8_t*>(buf);
		for (uint32_t col = 0; col < width; col++)
		{
			uint8_t colorIndex;
			if (bitsPerSample == 8)
			{
				colorIndex = in[col*samplesPerPixel];
			}
			else // bitspersample == 4 --- TESTME !
			{
				int offset = col*samplesPerPixel;
				if (offset & 0x01)
					colorIndex = (in[offset >> 1] & 0xf);
				else
					colorIndex = (in[offset >> 1] & 0xf0) >> 4;
			}
			if ( is16bit )
				reinterpret_cast<Image::Pixel_RGB_16*>(dataLine)[col] = colorMap[colorIndex];
			else
				reinterpret_cast<Image::Pixel_RGB_8*>(dataLine)[col] = colorMap_8bpc[colorIndex];
		}
	}
	_TIFFfree(buf);

}
