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

typedef enum BM_COMPRESSION { BI_RGB=0, BI_RLE8=1, BI_RLE4=2, BI_BITFIELDS=3 } BM_COMPRESSION;


static void readLineRLE( std::ifstream & file, BM_COMPRESSION biCompression, char* buffer, size_t bufferSize /*stride!*/ )
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
				for ( int i=nextPixelsUncompressed; i > 0; i-=2 )
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



void ImageCoderBitmap::read( const std::string & filename )
{
	bool loadGrayImagesAsGray = false; 	// todo: make me a flag!

	std::ifstream file( filename.c_str(), std::ios_base::binary );
	if ( file.fail() )
		throw std::runtime_error( "failed to open BMP file " + filename );

	getIccProfile().clear();

	// Bitmap File Header
	uint16_t bfType;	// "BM"
	uint32_t bfSize;
	uint32_t bfReserved;
	uint32_t bfOffBytes;
	file.read( reinterpret_cast<char*>( &bfType ), 2 );
	file.read( reinterpret_cast<char*>( &bfSize ), 4 );
	file.read( reinterpret_cast<char*>( &bfReserved ), 4 );
	file.read( reinterpret_cast<char*>( &bfOffBytes ), 4 );
	if ( bfType != 0x4d42 || bfReserved!= 0 )
		throw std::runtime_error( "failed to read BMP file " + filename + " - bogus BitmapFileHeader" );
	// don't rely on size!

	// Bitmap Info Header
	uint32_t biSize;	// size of BIH in bytes ...
	int32_t biWidth;
	int32_t biHeight;
	uint16_t biPlanes;
	uint16_t biBitCount;
	uint32_t biCompression = 0; // uncompressed
	uint32_t biSizeImage;
	int32_t  biXPelsPerMeter = static_cast<int32_t>( image->getResolutionX() / 2.54 * 100.0 + 0.5 );
	int32_t  biYPelsPerMeter = static_cast<int32_t>( image->getResolutionY() / 2.54 * 100.0 + 0.5 );
	uint32_t biClrUsed;
	uint32_t biClrImportant = 0;
	
	// read 40 bytes header = version 3 header
	file.read( reinterpret_cast<char*>( &biSize ), 4 );
	if ( biSize < 40 )
		throw std::runtime_error( "failed to read BMP file " + filename + " - bogus BitmapInfoHeader" );
	file.read( reinterpret_cast<char*>( &biWidth ), 4 );
	file.read( reinterpret_cast<char*>( &biHeight ), 4 );
	file.read( reinterpret_cast<char*>( &biPlanes ), 2 );
	file.read( reinterpret_cast<char*>( &biBitCount ), 2 );
	file.read( reinterpret_cast<char*>( &biCompression ), 4 );
	file.read( reinterpret_cast<char*>( &biSizeImage ), 4 );
	file.read( reinterpret_cast<char*>( &biXPelsPerMeter ), 4 );
	file.read( reinterpret_cast<char*>( &biYPelsPerMeter ), 4 );
	file.read( reinterpret_cast<char*>( &biClrUsed ), 4 );
	file.read( reinterpret_cast<char*>( &biClrImportant ), 4 );
	
	//////////////////////////////
	// analyze header

	
	// header version...
	int BIHeaderVersion = 0;
	if ( biSize >= 40 )
		BIHeaderVersion = 3;	// BITMAPINFOHEADER
	if ( biSize >= 108 )
		BIHeaderVersion = 4;	// BITMAPV4HEADER (contains potential color space correction --- which we don't want to use!)
	if ( biSize >= 124 )
		BIHeaderVersion = 5;	// BITMAPV5HEADER (contains potential icc profile --- not actually supported yet, typically only microsoft-internally used)
	// planes
	if ( biPlanes != 1 )
		throw std::runtime_error( "failed to read BMP file " + filename + " - bogus BitmapInfoHeader" );
	// size and orientation
	bool isBottomToTop = true;
	size_t width, height;
	if ( biWidth <= 0 || biHeight == 0 ) // not that negative height is actually allowed ! it means top-down bitmap (otherwise bottom up)
		throw std::runtime_error( "failed to read BMP file " + filename + " - bogus BitmapInfoHeader - invalid size" );
	width = biWidth;
	if ( biHeight < 0 )
	{
		isBottomToTop = false;
		height = -biHeight;
	}
	else
		height = biHeight;
		
	// BITCOUNT and color table
	bool hasIndexColors = false;
	size_t numColorTableEntries = 0;
	if ( biBitCount == 1 ) // 1,4,8 means indexed colors!
	{
		hasIndexColors = true;
		numColorTableEntries = 2;
	}
	else if ( biBitCount == 4 ) 
	{
		hasIndexColors = true;
		numColorTableEntries = biClrUsed==0 ? 16 : biClrUsed;
	}
	else if ( biBitCount == 8 ) 
	{
		hasIndexColors = true;
		numColorTableEntries = biClrUsed==0 ? 256 : biClrUsed;
	}
	else if ( biBitCount != 16 && biBitCount != 24 && biBitCount != 32 ) 
		throw std::runtime_error( "failed to read BMP file " + filename + " - bogus BitmapInfoHeader - invalid bitCount" );
	// ignore biClrImportant ...

	// resolution
	double dpiX = 96.0;
	double dpiY = 96.0;
	if ( biXPelsPerMeter != 0 )
		dpiX = dpiY = biXPelsPerMeter / 100.0 * 2.54;
	if ( biYPelsPerMeter != 0 )
		dpiY = biYPelsPerMeter / 100.0 * 2.54;

	// COMPRESSION 
	if ( biCompression == BI_RLE8 && ( biBitCount != 8 || biHeight < 0 ) )
		throw std::runtime_error( "failed to read BMP file " + filename + " - bogus BitmapInfoHeader - invalid compression RLE_8 / bitcount / imageHeight combination" );
	if ( biCompression == BI_RLE4 && ( biBitCount != 4 || biHeight < 0 ) )
		throw std::runtime_error( "failed to read BMP file " + filename + " - bogus BitmapInfoHeader - invalid compression RLE_4 / bitcount / imageHeight combination" );
	if ( biCompression == BI_BITFIELDS && biBitCount != 16 && biBitCount != 32 )
		throw std::runtime_error( "failed to read BMP file " + filename + " - bogus BitmapInfoHeader - invalid compression Mask / bitcount" );
	
	size_t numBytesHeaderRead = 40;
	
	// BitMasks.
	// - in case of V3 header, this comes AFTER the header for BI_BITFIELDS. (R;G;B)
	// - in vase of V4 and V5 header, this is PART of the header (and includes alpha

	enum BitMaskPlane { BM_RED=0, BM_GREEN=1, BM_BLUE=2, BM_ALPHA=3 };
	uint32_t bitMask[4] = { 0, 0, 0, 0 };
	bool hasAlpha;
	if ( biCompression == BI_BITFIELDS )
	{
		file.read( reinterpret_cast<char*>( &bitMask[BM_RED] ), 4 );
		file.read( reinterpret_cast<char*>( &bitMask[BM_GREEN] ), 4 );
		file.read( reinterpret_cast<char*>( &bitMask[BM_BLUE] ), 4 );
		numBytesHeaderRead += 12;
		if ( BIHeaderVersion >= 4 )
		{
			file.read( reinterpret_cast<char*>( &bitMask[BM_ALPHA] ), 4 );
			numBytesHeaderRead += 4;
		}
		if ( biBitCount == 16 && ( ( bitMask[BM_RED] & 0xffff0000 ) != 0 || ( bitMask[BM_GREEN] & 0xffff0000 ) != 0 || ( bitMask[BM_BLUE] & 0xffff0000 ) != 0 ) )
			throw std::runtime_error( "failed to read BMP file " + filename + " - bogus bitmasks for 16 bit encoding" );
	}
	else if ( biCompression == BI_RGB && biBitCount == 16 )
	{
		bitMask[BM_RED]   = 0x00007c00;
		bitMask[BM_GREEN] = 0x000003e0;
		bitMask[BM_BLUE]  = 0x0000001f;
	}
	else if ( biCompression == BI_RGB && biBitCount == 24 )
	{
		bitMask[BM_RED]   = 0x00ff0000;
		bitMask[BM_GREEN] = 0x0000ff00;
		bitMask[BM_BLUE]  = 0x000000ff;
	}
	else if ( biCompression == BI_RGB && biBitCount == 32 )
	{
		bitMask[BM_RED]   = 0x00ff0000;
		bitMask[BM_GREEN] = 0x0000ff00;
		bitMask[BM_BLUE]  = 0x000000ff;
		bitMask[BM_ALPHA] = 0xff000000;	// not actually specified, but used by adobe!
	}
	hasAlpha = bitMask[BM_ALPHA] != 0;
	
	// determine bitshifts
	uint32_t bitShift[4] = { 0, 0, 0, 0 };
	for ( int iIndex=BM_RED; iIndex<=BM_ALPHA; iIndex++ )
	{
		if ( bitMask[iIndex] )
			for ( int bitShiftBit=0; ((bitMask[iIndex]>>bitShiftBit)&0x1)==0; bitShiftBit++ )
				bitShift[iIndex] = bitShiftBit+1;
	}

	// skip the rest of the header
	if ( numBytesHeaderRead < biSize )
	{
		file.seekg( biSize - numBytesHeaderRead, std::ios_base::cur );
	}
	if ( file.fail() )
		throw std::runtime_error( "failed to read BMP file " + filename + " - IO-Error reading header (file corrupt ?)" );

	
	// not sure if I understand where this is located in case of V4 and V5 headers !!!!

	std::vector<Image::Pixel_RGB_8> colorTable;
	bool isGrayOnly = false;
	if ( hasIndexColors )
	{
		// Read Color Table and determine if it is gray only!
		colorTable.resize( numColorTableEntries );
		isGrayOnly = loadGrayImagesAsGray; // if the latter is false, this always stays false...
		for ( size_t indexedColor = 0; indexedColor < numColorTableEntries; indexedColor++ )
		{
			Image::Pixel_BGRA_8 entry;
			file.read( reinterpret_cast<char*>( &entry ), 4 );
			if ( isGrayOnly && ( entry.r != entry.g && entry.r != entry.b ) )
				isGrayOnly = false;
			colorTable[indexedColor].r = entry.r;
			colorTable[indexedColor].g = entry.g;
			colorTable[indexedColor].b = entry.b;
		}
	}

	// create the image
	PixelMode pm = PixelMode::create( false, !isGrayOnly, isGrayOnly, hasAlpha, 8 );
	image->create( width, height, pm, dpiX, dpiY );

	uint32_t stride = biWidth * biBitCount / 8;
	if ( stride % 4 ) stride += 4 - ( stride % 4 );

	// currently not supported: RLE (biCompression = 1, 2)
//	if ( biCompression >= 1 && biCompression <= 2 )
//		throw std::runtime_error( "failed to read BMP file " + filename + " - do not support RLE compression" );

	// go to beginning of data
	file.seekg( bfOffBytes );
	std::unique_ptr<char[]> lineBuffer( new char[stride] );
	for ( size_t row = 0; row < height; row++ )
	{
		void* intLine = image->getLine( isBottomToTop ? ( height - row - 1 ) : row );

		if ( biCompression == BI_RLE4 || biCompression == BI_RLE8 )
		{
			readLineRLE( file, static_cast<BM_COMPRESSION>(biCompression), lineBuffer.get(), stride );
		}
		else
		{
			file.read( lineBuffer.get(), stride );	
		}
		if ( file.fail() )
			throw std::runtime_error( "failed to read BMP file " + filename + " - premature end of data stream" );
		if ( hasIndexColors )
		{
			for ( size_t col = 0; col < width; col++ )
			{
				unsigned char index = getColorIndex( reinterpret_cast<unsigned char*>( lineBuffer.get() ), col, biBitCount );
				if ( isGrayOnly )
					reinterpret_cast<Image::Pixel_GRAY_8*>( intLine )[col].value = colorTable[index].r;
				else
					reinterpret_cast<Image::Pixel_RGB_8*>( intLine )[col] = colorTable[index];	// geht dem ?
			}
		}
		else
		{
			// 16, 24 or 32 bit mode
			// extract data using bitMask and bitShift from line...
			uint8_t* dataBMP = reinterpret_cast<uint8_t*>(lineBuffer.get());
			uint32_t pixelVal;
			for ( size_t col = 0; col < width; col++ )
			{
				switch ( biBitCount )
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
				default:
					throw std::runtime_error( "failed to read BMP file " + filename +": Illegal bitCount for non-indexed image: " + std::to_string(biBitCount) );
				}
				if ( hasAlpha )
				{
					Image::Pixel_ARGB_8* data = reinterpret_cast<Image::Pixel_ARGB_8*>( intLine );
					data[col].a = (pixelVal & bitMask[BM_ALPHA]) >> bitShift[BM_ALPHA];
					data[col].r = (pixelVal & bitMask[BM_RED]) >> bitShift[BM_RED];
					data[col].g = (pixelVal & bitMask[BM_GREEN]) >> bitShift[BM_GREEN];
					data[col].b = (pixelVal & bitMask[BM_BLUE]) >> bitShift[BM_BLUE];					
				}
				else
				{
					Image::Pixel_RGB_8* data = reinterpret_cast<Image::Pixel_RGB_8*>( intLine );
					data[col].r = (pixelVal & bitMask[BM_RED]) >> bitShift[BM_RED];
					data[col].g = (pixelVal & bitMask[BM_GREEN]) >> bitShift[BM_GREEN];
					data[col].b = (pixelVal & bitMask[BM_BLUE]) >> bitShift[BM_BLUE];
				}
			}
		}
	}

	onSubImageRead( 1 );	// we have read image #1
	onSubImageRead( 0 );	// we are done!
}

void ImageCoderBitmap::write( const std::string & filename )
{
	std::ofstream file( filename.c_str(), std::ios_base::binary|std::ios_base::trunc );
	if ( file.fail() )
		throw std::runtime_error( "failed to create BMP file " + filename );

	uint32_t stride;

	// Bitmap File Header
	uint16_t bfType = 0x4d42;	// "BM"
	uint32_t bfSize;
	uint32_t bfReserved = 0;
	uint32_t bfOffBytes;

	// Bitmap Info Header
	uint32_t biSize = 40;	// size of BIH in bytes ...
	int32_t biWidth = static_cast<int32_t>( image->getWidth() );
	int32_t biHeight = static_cast<int32_t>( image->getHeight() );
	uint16_t biPlanes = 1;
	uint16_t biBitCount;
	uint32_t biCompression = 0; // uncompressed
	uint32_t biSizeImage;
	int32_t  biXPelsPerMeter = static_cast<int32_t>( image->getResolutionX() / 2.54 * 100.0 + 0.5 );
	int32_t  biYPelsPerMeter = static_cast<int32_t>( image->getResolutionY() / 2.54 * 100.0 + 0.5 );
	uint32_t biClrUsed;
	uint32_t biClrImportant = 0;

	switch ( image->getPixelMode() )
	{
	case PixelMode::GRAY8:
		biBitCount = 8; // 8 bit indexed!
		stride = biWidth;
		if ( stride % 4 ) stride += 4 - ( stride % 4 );
		biSizeImage = stride * biHeight;
		biClrUsed = 256;
		break;
	case PixelMode::RGB8:
		biBitCount = 24; // 24 byte RGB
		stride = biWidth * 3;
		if ( stride % 4 ) stride += 4 - ( stride % 4 );
		biSizeImage = stride * biHeight;
		biClrUsed = 0;
		break;
	default:
		throw std::runtime_error( "Image can't be saved as BMP!" );
	}

	bfOffBytes = 14 + 40 + biClrUsed * 4;
	bfSize = bfOffBytes + biSizeImage;
	file.write( reinterpret_cast<char*>( &bfType ), 2 );
	file.write( reinterpret_cast<char*>( &bfSize ), 4 );
	file.write( reinterpret_cast<char*>( &bfReserved ), 4 );
	file.write( reinterpret_cast<char*>( &bfOffBytes ), 4 );

	file.write( reinterpret_cast<char*>( &biSize ), 4 );
	file.write( reinterpret_cast<char*>( &biWidth ), 4 );
	file.write( reinterpret_cast<char*>( &biHeight ), 4 );
	file.write( reinterpret_cast<char*>( &biPlanes ), 2 );
	file.write( reinterpret_cast<char*>( &biBitCount ), 2 );
	file.write( reinterpret_cast<char*>( &biCompression ), 4 );
	file.write( reinterpret_cast<char*>( &biSizeImage ), 4 );
	file.write( reinterpret_cast<char*>( &biXPelsPerMeter ), 4 );
	file.write( reinterpret_cast<char*>( &biYPelsPerMeter ), 4 );
	file.write( reinterpret_cast<char*>( &biClrUsed ), 4 );
	file.write( reinterpret_cast<char*>( &biClrImportant ), 4 );

	switch ( image->getPixelMode() )
	{
	case PixelMode::GRAY8:
		// color table
		for ( int gray = 0; gray < 256; gray++ )
		{
			uint32_t val = gray | ( gray << 8 ) | ( gray << 16 );
			file.write( reinterpret_cast<char*>( &val ), 4 );
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
				file.write( reinterpret_cast<char*>( image->getLine( line ) ), std::min( stride,strideInData ) );
				if ( extraStride )
					file.write( reinterpret_cast<char*>( &nil ), extraStride );
			}
		}
		break;
	case PixelMode::RGB8:
		// no color table
	{
		std::unique_ptr<char[]> outBuffer( new char[stride] );
		std::memset( outBuffer.get(), 0, stride ); // to make sure padding bytes are 0 and avoid valgrind complains
		for ( int line = image->getHeight() - 1; line >= 0; line-- )
		{
			Image::Pixel_RGB_8* data = reinterpret_cast<Image::Pixel_RGB_8*>( image->getLine( line ) );
			Image::Pixel_BGR_8* dataOut = reinterpret_cast<Image::Pixel_BGR_8*>( outBuffer.get() );
			for ( size_t x = 0; x < image->getWidth(); x++ )
			{
				dataOut[x].r = data[x].r;
				dataOut[x].g = data[x].g;
				dataOut[x].b = data[x].b;
			}
			file.write( reinterpret_cast<char*>( dataOut ), stride );
		}
	}
	break;
	default:
		throw std::runtime_error( "Image can't be saved as BMP!" );
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
