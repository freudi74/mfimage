/* ImageCoderBitmap.cpp
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
#include "ImageCoderBitmap.h"
#include <fstream>
#include <algorithm>
#include <memory>


#pragma pack(push,2)
// Bitmap File Header
typedef struct BitmapFileHeader {
	uint16_t type;	// "BM"
	uint32_t size;
	uint32_t reserved;
	uint32_t offsetBytes;
} BitmapFileHeader; // size: 14 bytes
#pragma pack(pop)

#pragma pack(push,4)
typedef struct BitmapInfoHeaderV5 {
	uint32_t size;	// size of BIH in bytes ...
	int32_t  width;
	int32_t  height;
	uint16_t planes;
	uint16_t bitCount;
	uint32_t compression = 0; // uncompressed
	uint32_t sizeImage;
	int32_t  xPelsPerMeter; //= static_cast<int32_t>(image->getResolutionX() / 2.54 * 100.0 + 0.5);
	int32_t  yPelsPerMeter; //= static_cast<int32_t>(image->getResolutionY() / 2.54 * 100.0 + 0.5);
	uint32_t clrUsed;
	uint32_t clrImportant = 0;
	// 40 bytes till including above - V1

	uint32_t redMask = 0x00ff0000;
	uint32_t greenMask = 0x0000ff00;
	uint32_t blueMask = 0x000000ff;
	// 52 bytes till including above - V2

	uint32_t alphaMask = 0xff000000;
	// 56 bytes till including above - V3

	uint32_t csType = 0x73524742;	// 0 = calibrated rgb - use endpoints and gamma, 0x73524742 ('sRGB'): sRGB!, 0x57696E20 ('Win '): Windows default color space (whatever that means ...)
	// endpooint. only used if csType = 0. Note that this is NOT xyY !!!! the formular to convert is: x = X/(X+Y+Z); y = Y/(Y+Y+Z); Y==Y
	uint32_t endpoint_ciexyzRed_x;	// this is actually a FXPT2DOT30 (2.30) 
	uint32_t endpoint_ciexyzRed_y;	// this is actually a FXPT2DOT30 (2.30)
	uint32_t endpoint_ciexyzRed_z;	// this is actually a FXPT2DOT30 (2.30)
	uint32_t endpoint_ciexyzGreen_x;	// this is actually a FXPT2DOT30 (2.30)
	uint32_t endpoint_ciexyzGreen_y;	// this is actually a FXPT2DOT30 (2.30)
	uint32_t endpoint_ciexyzGreen_z;	// this is actually a FXPT2DOT30 (2.30)
	uint32_t endpoint_ciexyzBlue_x;	// this is actually a FXPT2DOT30 (2.30)
	uint32_t endpoint_ciexyzBlue_y;	// this is actually a FXPT2DOT30 (2.30)
	uint32_t endpoint_ciexyzBlue_z;	// this is actually a FXPT2DOT30 (2.30)
	uint32_t gammaRed = 0;	// 16.16 unsigned
	uint32_t gammaGreen = 0;
	uint32_t gammaBlue = 0;
	// 108 bytes till including above - V4

	uint32_t intent = 0; // 1=saturation; 2=relColorimetric; 4=perceptual; 8=absColorimetric
	uint32_t profileDataOffset = 0; // the offset, in bytes, from the beginning of the BITMAPV5HEADER structure to the start of the profile data.
	uint32_t profileDataSize = 0; // the size of the profile
	uint32_t reserved = 0;
	// 124 bytes till including above - V5

} BitmapInfoHeaderV5;
#pragma pack(pop)

ImageCoderBitmap::ImageCoderBitmap( Image* img ) : ImageCoder( IE_BMP, img )
{
}

ImageCoderBitmap::~ImageCoderBitmap()
{
}

bool ImageCoderBitmap::canDecode()
{
	return true;
}

bool ImageCoderBitmap::canEncode()
{
	return true;
}

bool ImageCoderBitmap::canEncode( PixelMode pixelMode )
{
	switch ( pixelMode ) {
	case PixelMode::GRAY8:   return true;
	case PixelMode::GRAY16:  
	case PixelMode::AGRAY8:  
	case PixelMode::AGRAY16: break;
	case PixelMode::RGB8:    return true;
	case PixelMode::RGB16:   
	case PixelMode::ARGB8:   
	case PixelMode::ARGB16:  
	case PixelMode::CMYK8:   
	case PixelMode::CMYK16:  
	case PixelMode::CMYKA8:  
	case PixelMode::CMYKA16: break;
	// no default to get compiler warning
	}	
	return false;
}

typedef enum BM_COMPRESSION { BI_RGB=0, BI_RLE8=1, BI_RLE4=2, BI_BITFIELDS=3, BI_JPEG=4, BI_PNG=5 } BM_COMPRESSION;


static void readLineRLE( std::istream & file, BM_COMPRESSION biCompression, char* buffer, size_t bufferSize /*stride!*/ )
{
	// FROM: Wikipedia.de
	// BI_RLE8 und BI_RLE4
	// Jeweils zwei aufeinanderfolgende Bytes bilden einen Datensatz. 
	// Hat das erste Byte einen anderen Wert als 0, so wird das zweite Byte so oft (bei BI_RLE4: die nächsten 2 Nibbles insgesamt, Beispiel: 05 67 → 6 7 6 7 6) wiederholt, wie das erste Byte angibt. 
	// Hat das erste Byte hingegen den Wert 0, so hängt die Bedeutung vom zweiten Byte ab:
	// 0: 	Ende der Bildzeile.
    // 1: 	Ende der Bitmap.
    // 2: 	Verschiebung der aktuellen Pixelposition. Die beiden nächsten Bytes geben die Verschiebung nach rechts und nach unten an.
    // n=3-255: 	Die folgenden n Bytes (bei BI_RLE4: die folgenden n Nibbles) werden direkt übernommen; der nächste Datensatz findet sich am darauffolgenden geraden Offset (vom Start der Bilddaten aus gezählt).
	// Das Resultat wird wie im unkomprimierten Fall interpretiert. 
	uint8_t record[2];
	char* bufferEnd = buffer+bufferSize;
	bool bufferInSecondNibble = false;
	
	auto writeNibble = [&buffer,&bufferInSecondNibble]( uint8_t nibble ) { 
		if ( bufferInSecondNibble ) {
			*buffer |= (nibble & 0xf);
			bufferInSecondNibble = false;
			buffer++;
		}
		else
		{
			*buffer = (nibble & 0xf) << 4;
			bufferInSecondNibble = true;
		}
	};
	
	while ( 1 )
	{
		file.read( reinterpret_cast<char*>(record), 2 );

		// ANALYZE RECORD
		size_t nextPixelsUncompressed = 0;
		size_t repeatPixels = 0;
		
		if ( record[0] )
		{
			repeatPixels = record[0];
		}
		else if ( record[1] <= 1 )
		{
			break; // end of line or end of bitmap
		}
		else if ( record[1] == 2 )
		{
			// "Verschiebung der aktuellen Pixelposition. Die beiden nächsten Bytes geben die Verschiebung nach rechts und nach unten an."
			// no idea what that means, actually - and German is my mother's tongue ...
			throw std::runtime_error( "RLE4/8 byte 2 = 2 --- not implemented as the author did not understand what that means" );
		}
		else if ( record[1] > 2 )
		{
			nextPixelsUncompressed = record[1];
		}
		
		// EXECUTE RECORD
		if ( biCompression == BI_RLE4 )
		{
			
			if ( nextPixelsUncompressed )
			{
				char pixel;
				for ( int i=static_cast<int>(nextPixelsUncompressed); i > 0; i-=2 )
				{
					file.read( &pixel, 1 );
					writeNibble( pixel >> 4 );
					if ( i > 1 )
						writeNibble( pixel & 0xf );
				}
				file.seekg( nextPixelsUncompressed / 2, std::ios_base::cur ); // not sure if we might not need to skip to next even number of BYTES... so we might need to skip 1 more byte in those cases actually. Need more test files to verify!
			}
			else if ( repeatPixels )
			{
				uint8_t nibble[2];
				nibble[0] = record[1] >> 4;
				nibble[1] = record[1] & 0xf;
				for ( size_t i=0; i<repeatPixels; i++ )
					writeNibble( nibble[i&0x01] );
			}
		}
		else if ( biCompression == BI_RLE8 )
		{
			if ( nextPixelsUncompressed )
			{
				file.read( buffer, nextPixelsUncompressed );
				buffer += nextPixelsUncompressed;
				if ( nextPixelsUncompressed & 0x1 )
					file.seekg( 1, std::ios_base::cur );
					
			}
			else if ( repeatPixels )
			{
				for ( size_t i=0; i<repeatPixels; i++ )
					*buffer++ = record[1];
			}
		}
		if ( buffer > bufferEnd ) 
		{
			throw std::runtime_error( "buffer overflow during BMP_RLE!" );
		}
	}
}

static uint32_t determineBitshift(uint32_t bitmask)
{
	uint32_t shift = 0;
	if (bitmask) {
		for (int bitShift = 0; ((bitmask >> bitShift) & 0x1) == 0; bitShift++)
			shift = bitShift + 1;
	}
	return shift;
}

void ImageCoderBitmap::read( const std::string & filename )
{
	std::ifstream file( filename.c_str(), std::ios_base::binary );
	if ( file.fail() )
		throw std::runtime_error( "failed to open BMP file " + filename );

	read(file);
}

typedef struct Pixel_BGRA_8   { uint8_t  b; uint8_t  g; uint8_t  r; uint8_t  a; } Pixel_BGRA_8;		// only for external usage. Internal always ARGB.
//	typedef struct Pixel_BGRA_16  { uint16_t b; uint16_t g; uint16_t r; uint16_t a; } Pixel_BGRA_16;	// only for external usage. Internal always ARGB.
typedef struct Pixel_BGR_8    { uint8_t  b; uint8_t  g; uint8_t  r;             } Pixel_BGR_8;		// only for external usage. Internal always RGB.
//	typedef struct Pixel_BGR_16   { uint16_t b; uint16_t g; uint16_t r;             } Pixel_BGR_16;		// only for external usage. Internal always RGB.


void ImageCoderBitmap::read(std::istream & stream)
{
	bool loadGrayImagesAsGray = false; 	// todo: make me a flag!

	getIccProfile().clear();

	BitmapFileHeader bf;

	stream.read(reinterpret_cast<char*>(&bf), 14);
	if (bf.type != 0x4d42 || bf.reserved != 0)
		throw std::runtime_error("failed to read BMP file - bogus BitmapFileHeader");
	// don't rely on size is what multiple documentations say!

	// Bitmap Info Header
	BitmapInfoHeaderV5 bi;
	size_t bitmapInfoHeaderVersion = 0;

	// read 40 bytes header = bmp version 3 header (BITMAPINFOHEADERV1!)
	stream.read(reinterpret_cast<char*>(&bi), 40);
	if ( stream.fail() )
		throw std::runtime_error("failed to read BMP file - bogus BitmapInfoHeader. Not a Windows V3.x or newer BMP file. Windows V2.x or OS/2 1.x Bitmap?");
	// read the rest of the header ...
	if (bi.size > 40) 
	{
		size_t headerbyteLeft = std::min(124u, bi.size) - 40u;
		stream.read(reinterpret_cast<char*>(&bi) + 40, headerbyteLeft);
		if (stream.fail())
			throw std::runtime_error("failed to read BMP file - IO-Error reading header (file corrupt ?)");
	}

	//////////////////////////////
	// analyze header version
	switch (bi.size) 
	{
	case 40:	
		bitmapInfoHeaderVersion = 1; // BITMAPINFOHEADER (BMP V3 and newer --- don't confuse V3 with BITMAPV3HEADER!)
		break;	
	case 52:	
		bitmapInfoHeaderVersion = 2; // Undocumented BITMAPV2HEADER (not listed in MSDN anymore - was it once? seems to contain masks for R,G,B)
		break;
	case 56:
		bitmapInfoHeaderVersion = 3; // BITMAPV3HEADER (not listed in current MSDN anymore, contains masks for R,G, B, A)
		break;
	case 64:
		throw std::runtime_error("failed to read BMP file - bogus BitmapInfoHeader. This is likely an OS/2 Bitmap, not a Windows Bitmap!");
	case 108:
		bitmapInfoHeaderVersion = 4; // BITMAPV4HEADER (contains potential color space correction --- which we don't use yet.)
		break;
	case 124:
		bitmapInfoHeaderVersion = 5; // BITMAPV4HEADER (contains potential icc profile --- not actually supported yet, typically only microsoft-internally used)
		break;
	default:
		if (bi.size < 124) 
		{
			throw std::runtime_error("failed to read BMP file - bogus BitmapInfoHeader. Unsupported bitmap info header version!");
		}
		bitmapInfoHeaderVersion = 5; // this seems to be a newer version, we take what we know.
		break;
	}


	// planes
	if (bi.planes != 1)
		throw std::runtime_error("failed to read BMP file - bogus BitmapInfoHeader");

	// size and orientation
	bool isBottomToTop = true;
	size_t width, height;
	if (bi.width <= 0 || bi.height == 0) // not that negative height is actually allowed ! it means top-down bitmap (otherwise bottom up)
		throw std::runtime_error("failed to read BMP file - bogus BitmapInfoHeader - invalid image size");
	width = bi.width;
	if (bi.height < 0)
	{
		isBottomToTop = false;
		height = -bi.height;
	}
	else
		height = bi.height;

	// BITCOUNT and color table
	bool hasIndexColors = false;
	size_t numColorTableEntries = 0;
	switch (bi.bitCount) {
	case 1: // 1,4,8 means indexed colors!
		hasIndexColors = true;
		numColorTableEntries = 2;
		break;
	case 4:
		hasIndexColors = true;
		numColorTableEntries = bi.clrUsed == 0 ? 16 : bi.clrUsed;
		break;
	case 8:
		hasIndexColors = true;
		numColorTableEntries = bi.clrUsed == 0 ? 256 : bi.clrUsed;
		break;
	case 16:
	case 24:
	case 32:
		// non-indexed colors, use masks
		break;
	default:
		// 
		throw std::runtime_error("failed to read BMP file - bogus BitmapInfoHeader - invalid bitCount");
	}

	// we ignore bi.clrImportant ...

	// resolution
	double dpiX = 96.0;
	double dpiY = 96.0;
	if (bi.xPelsPerMeter != 0)
		dpiX = dpiY = bi.xPelsPerMeter / 100.0 * 2.54;
	if (bi.yPelsPerMeter != 0)
		dpiY = bi.yPelsPerMeter / 100.0 * 2.54;

	// COMPRESSION, Bitmasks, etc
	switch (bi.compression) 
	{
	case BI_RGB:
		switch (bi.bitCount) 
		{
		case 1:
		case 4:
		case 8:
			// indexed colors ...
			bi.redMask = 0;
			bi.greenMask = 0;
			bi.blueMask = 0;
			bi.alphaMask = 0;
			break;
		case 16:
			// overwrite masks in file
			bi.redMask = 0x00007c00;
			bi.greenMask = 0x000003e0;
			bi.blueMask = 0x0000001f;
			bi.alphaMask = 0;
			break;
		case 24:
			// overwrite masks in file
			bi.redMask = 0x00ff0000;
			bi.greenMask = 0x0000ff00;
			bi.blueMask = 0x000000ff;
			bi.alphaMask = 0;
			break;
		case 32:
			// overwrite masks in file
			bi.redMask = 0x00ff0000;
			bi.greenMask = 0x0000ff00;
			bi.blueMask = 0x000000ff;
			bi.alphaMask = 0xff000000;	// not actually specified, but used by adobe!
			break;
		default:
			throw std::runtime_error("failed to read BMP file - bogus BitmapInfoHeader - invalid compression Mask / bitCount for RGB image");
		}
		break;
	case BI_RLE8:
		if ( bi.bitCount != 8 || bi.height < 0 )
			throw std::runtime_error("failed to read BMP file - bogus BitmapInfoHeader - invalid compression RLE_8 / bitCount / imageHeight combination");
		bi.alphaMask = 0;
		break;
	case BI_RLE4:
		if (bi.bitCount != 4 || bi.height < 0)
			throw std::runtime_error("failed to read BMP file - bogus BitmapInfoHeader - invalid compression RLE_4 / bitCount / imageHeight combination");
		bi.alphaMask = 0;
		break;
	case BI_BITFIELDS:
		if (bi.bitCount != 16 || bi.bitCount != 32 )
			throw std::runtime_error("failed to read BMP file - bogus BitmapInfoHeader - invalid compression Mask / bitCount");
		if (bitmapInfoHeaderVersion < 2) {
			// - in case of V1 header, this comes AFTER the header for BI_BITFIELDS. (R;G;B)
			// - in all other cases, it's PART of the header ...
			stream.read(reinterpret_cast<char*>(&bi.redMask), 4);
			stream.read(reinterpret_cast<char*>(&bi.greenMask), 4);
			stream.read(reinterpret_cast<char*>(&bi.blueMask), 4);
		}
		if (bitmapInfoHeaderVersion < 3) {
			bi.alphaMask = 0; // otherwise as in file.
		}
		if (bi.bitCount == 16 && ((bi.redMask & 0xffff0000) != 0 || (bi.greenMask & 0xffff0000) != 0 || (bi.blueMask & 0xffff0000) != 0 || (bi.alphaMask & 0xffff0000) != 0))
			throw std::runtime_error("failed to read BMP file - bogus bitmasks for 16 bit encoding");
		break;
	case BI_JPEG:
		bi.alphaMask = 0;
		throw std::runtime_error("failed to read BMP file - Compression type JPEG not supported by " MFIMAGELIB_LIB_NAME );
	case BI_PNG:
		bi.alphaMask = 0;
		throw std::runtime_error("failed to read BMP file - Compression type PNG not supported by " MFIMAGELIB_LIB_NAME );
	default:
		bi.alphaMask = 0;
		throw std::runtime_error("failed to read BMP file - Unknown compression type encountered");
	}
	
	bool hasAlpha = bi.alphaMask != 0;

	//////////////////////////////////////////////////////////////////////////
	// determine bitshifts
	uint32_t bitShiftRed   = determineBitshift(bi.redMask);
	uint32_t bitShiftGreen = determineBitshift(bi.greenMask);
	uint32_t bitShiftBlue  = determineBitshift(bi.blueMask);
	uint32_t bitShiftAlpha = determineBitshift(bi.alphaMask);


	//////////////////////////////////////////////////////////////////////////
	// Read color table

	// not sure if I understand where this is located in case of V4 and V5 headers !!!!

	std::vector<Image::Pixel_RGB_8> colorTable;
	bool isGrayOnly = false;
	if (hasIndexColors)
	{
		// Read Color Table and determine if it is gray only!
		colorTable.resize(numColorTableEntries);
		isGrayOnly = loadGrayImagesAsGray; // if the latter is false, this always stays false...
		for (size_t indexedColor = 0; indexedColor < numColorTableEntries; indexedColor++)
		{
			Pixel_BGRA_8 entry;
			stream.read(reinterpret_cast<char*>(&entry), 4);
			if (isGrayOnly && (entry.r != entry.g && entry.r != entry.b))
				isGrayOnly = false;
			colorTable[indexedColor].r = entry.r;
			colorTable[indexedColor].g = entry.g;
			colorTable[indexedColor].b = entry.b;
		}
	}

	// create the image
	PixelMode pm = PixelMode::create(false, !isGrayOnly, isGrayOnly, false, hasAlpha, 8);
	image->create(width, height, pm, dpiX, dpiY);

	// calculate stride
	uint32_t stride = bi.width * bi.bitCount / 8;
	if (stride % 4) stride += 4 - (stride % 4);

	// go to beginning of data
	stream.seekg(bf.offsetBytes);
	std::unique_ptr<char[]> lineBuffer(new char[stride]);
	for (size_t row = 0; row < height; row++)
	{
		void* intLine = image->getLine(isBottomToTop ? (height - row - 1) : row);

		if (bi.compression == BI_RLE4 || bi.compression == BI_RLE8)
		{
			readLineRLE(stream, static_cast<BM_COMPRESSION>(bi.compression), lineBuffer.get(), stride);
		}
		else
		{
			stream.read(lineBuffer.get(), stride);
		}
		if (stream.fail())
			throw std::runtime_error("failed to read BMP file - premature end of data stream");
		if (hasIndexColors)
		{
			for (size_t col = 0; col < width; col++)
			{
				unsigned char index = getColorIndex(reinterpret_cast<unsigned char*>(lineBuffer.get()), col, bi.bitCount);
				if (isGrayOnly)
					reinterpret_cast<Image::Pixel_GRAY_8*>(intLine)[col].value = colorTable[index].r;
				else
					reinterpret_cast<Image::Pixel_RGB_8*>(intLine)[col] = colorTable[index];	// geht dem ?
			}
		}
		else
		{
			// 16, 24 or 32 bit mode
			// extract data using bitMask and bitShift from line...
			uint8_t* dataBMP = reinterpret_cast<uint8_t*>(lineBuffer.get());
			uint32_t pixelVal;
			for (size_t col = 0; col < width; col++)
			{
				switch (bi.bitCount)
				{
				case 16:
					pixelVal = *dataBMP++;
					pixelVal |= *dataBMP++ << 8;
					break;
				case 24:
					pixelVal = *dataBMP++;
					pixelVal |= *dataBMP++ << 8;
					pixelVal |= *dataBMP++ << 16;
					break;
				case 32:
					pixelVal = *dataBMP++;
					pixelVal |= *dataBMP++ << 8;
					pixelVal |= *dataBMP++ << 16;
					pixelVal |= *dataBMP++ << 24;
					break;
				}
				if (hasAlpha)
				{
					Image::Pixel_ARGB_8* data = reinterpret_cast<Image::Pixel_ARGB_8*>(intLine);
					data[col].alpha = (pixelVal & bi.alphaMask) >> bitShiftAlpha;
					data[col].r = (pixelVal & bi.redMask) >> bitShiftRed;
					data[col].g = (pixelVal & bi.greenMask) >> bitShiftGreen;
					data[col].b = (pixelVal & bi.blueMask) >> bitShiftBlue;
				}
				else
				{
					Image::Pixel_RGB_8* data = reinterpret_cast<Image::Pixel_RGB_8*>(intLine);
					data[col].r = (pixelVal & bi.redMask) >> bitShiftRed;
					data[col].g = (pixelVal & bi.greenMask) >> bitShiftGreen;
					data[col].b = (pixelVal & bi.blueMask) >> bitShiftBlue;
				}
			}
		}
	}

	onSubImageRead(1);	// we have read image #1
	onSubImageRead(0);	// we are done!
}

void ImageCoderBitmap::write(const std::string & filename)
{
	std::ofstream file(filename.c_str(), std::ios_base::binary | std::ios_base::trunc);
	if (file.fail())
		throw std::runtime_error("failed to create BMP file " + filename);

	write(file);
}

void ImageCoderBitmap::write(std::ostream & stream) 
{
	// Bitmap File Header
	BitmapFileHeader bf;
	bf.type = 0x4d42;	// "BM"
	bf.reserved = 0;

	// Bitmap Info Header
	BitmapInfoHeaderV5 bi;
	bi.size = 40;	// we currently only write a BitmapinfoheaderV1
	bi.width = static_cast<int32_t>( image->getWidth() );
	bi.height = static_cast<int32_t>( image->getHeight() );
	bi.planes = 1;
	bi.bitCount = 0;
	bi.compression = BI_RGB;
	bi.sizeImage = 0;
	bi.xPelsPerMeter = static_cast<int32_t>( image->getResolutionX() / 2.54 * 100.0 + 0.5 );
	bi.yPelsPerMeter = static_cast<int32_t>( image->getResolutionY() / 2.54 * 100.0 + 0.5 );
	bi.clrUsed = 0;
	bi.clrImportant = 0;

	uint32_t stride;
	switch (image->getPixelMode())
	{
	case PixelMode::GRAY8:
		// 8 bit indexed!
		stride = bi.width;
		if ( stride % 4 ) stride += 4 - ( stride % 4 );
		bi.bitCount     = 8; 
		bi.sizeImage    = stride * bi.height;
		bi.clrUsed      = 256;
		bi.clrImportant = 256;
		break;
	case PixelMode::RGB8:
		// 24 byte RGB
		stride = bi.width * 3;
		if ( stride % 4 ) stride += 4 - ( stride % 4 );
		bi.bitCount  = 24; 
		bi.sizeImage = stride * bi.height;
		break;
	default:
		throw std::runtime_error( "Image can't be saved as BMP!" ); // should have been caught by canEnconde
	}

	bf.offsetBytes = 14 + bi.size + bi.clrUsed * 4;	// file header, info header v1, color table
	bf.size = bf.offsetBytes + bi.sizeImage;
	stream.write( reinterpret_cast<char*>( &bf ), 14 );
	stream.write( reinterpret_cast<char*>( &bi ), bi.size );

	switch ( image->getPixelMode() )
	{
	case PixelMode::GRAY8:
		// color table
		for ( int gray = 0; gray < 256; gray++ )
		{
			uint32_t val = gray | ( gray << 8 ) | ( gray << 16 );
			stream.write( reinterpret_cast<char*>( &val ), 4 );
		}
		// pixels
		{
			uint32_t strideInData = image->getStride();
			uint32_t extraStride = 0;
			if ( stride > strideInData )
				extraStride = stride - strideInData;
			size_t nil = 0;
			for ( int line = image->getHeight() - 1; line >= 0; line-- )
			{
				stream.write(reinterpret_cast<char*>(image->getLine(line)), std::min(stride, strideInData));
				if ( extraStride )
					stream.write(reinterpret_cast<char*>(&nil), extraStride);
			}
		}
		break;
	case PixelMode::RGB8:
		// no color table
	{
		std::unique_ptr<char[]> outBuffer( new char[stride] );
		std::memset( outBuffer.get(), 0, stride ); // to make sure padding bytes are 0 and avoid valgrind complaints
		for ( int line = image->getHeight() - 1; line >= 0; line-- )
		{
			Image::Pixel_RGB_8* data = reinterpret_cast<Image::Pixel_RGB_8*>( image->getLine( line ) );
			Pixel_BGR_8* dataOut = reinterpret_cast<Pixel_BGR_8*>( outBuffer.get() );
			for ( size_t x = 0; x < image->getWidth(); x++ )
			{
				dataOut[x].r = data[x].r;
				dataOut[x].g = data[x].g;
				dataOut[x].b = data[x].b;
			}
			stream.write(reinterpret_cast<char*>(dataOut), stride);
		}
	}
	break;
	default:
		throw std::runtime_error( "Image can't be saved as BMP!" );  // should have been caught by canEnconde
	}

}

unsigned char ImageCoderBitmap::getColorIndex( const unsigned char* line, size_t pixelNo, uint32_t bitsPerPixel )
{
	// not sure if this is the right way. test !
	static unsigned char oneBitMask[8] = { 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01 };
	switch ( bitsPerPixel )
	{
	case 8:
		return line[pixelNo];
	case 4:
		// not sure if this is the right way. test !
		if ( pixelNo & 0x1 )
			return line[pixelNo >> 1] & 0xf;
		else
			return line[pixelNo >> 1] >> 4;
	case 1:
		return ( line[pixelNo >> 3] & oneBitMask[pixelNo & 0x7] ) ? 1 : 0;
	}
	return 0;	// never happens !
}
