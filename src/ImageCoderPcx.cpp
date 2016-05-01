/* ImageCoderPcx.cpp
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
#include "ImageCoderPcx.h"
#include <array>

ImageCoderPcx::ImageCoderPcx(Image* img) : ImageCoder(IE_PCX, img)
{
}


ImageCoderPcx::~ImageCoderPcx()
{
}

bool ImageCoderPcx::canEncode()
{
	return false;
}

bool ImageCoderPcx::canDecode()
{
	return true;
}
bool ImageCoderPcx::canEncode( PixelMode pixelMode )
{
	return false;
}

// this is not complete yet. Only tested for 24 bit (RGB*8) RLE files.
// certainly does not work right for any BPP != 8 in the moment (unless using VGA palette ...). Does not work for grayscale. 
// Might work for 8-bit VGA palette files, but not for EGA, CGA palette...
void ImageCoderPcx::read(const std::string & filename)
{
	std::ifstream file(filename.c_str(), std::ios_base::binary);
	if (file.fail())
		throw std::runtime_error("failed to open PCX file " + filename);

	read(file);
}

void ImageCoderPcx::read(std::istream & stream)
{
	getIccProfile().clear();

	// PCX File Header (128 bytes) --- see: http://www.fileformat.info/format/pcx/egff.htm
	uint8_t  pcxhId;	// 0x0a
	uint8_t  pcxhVer;	// PCX-File Version: 0 = Version 2.5 2 = Version 2.8 mit Palette 3 = Version 2.8 ohne Palette 4 = Windows ohne Palette 	5 = Version 3.0
	uint8_t  pcxhEnc;   // Encoding. 0 = raw; 1 = RLE
	uint8_t  pcxhBPP;   // Bits per pixel (or: plane)
	uint16_t pcxhXStart;
	uint16_t pcxhYStart;	
	uint16_t pcxhXEnd;		// Image Width = pcxhXEnd - pcxhXStart + 1;
	uint16_t pcxhYEnd;		// Image Height = pcxhYEnd - pcxhYStart + 1;
	uint16_t pcxhDpiX;
	uint16_t pcxhDpiY;
	Image::Pixel_RGB_8  pcxhEgaPalette[16]; // 16 color EGA palette RGB - 48 bytes
	uint8_t  pcxhReserved1;
	uint8_t  pcxhNumBitPlanes;	// number of bit planes.
	uint16_t pcxhBytesPerLine;
	uint16_t pcxhPaletteType;   // 1 = color or monochrome; 2 = grayscale.
	uint16_t pcxhHScreenSize;	// we don't care ..
	uint16_t pcxhVScreenSize;	// we don't care ..
	uint8_t  pcxhReserved2[54]; // 16 color EGA palette RGB
	stream.read(reinterpret_cast<char*>(&pcxhId), 1);
	stream.read(reinterpret_cast<char*>(&pcxhVer), 1);
	stream.read(reinterpret_cast<char*>(&pcxhEnc), 1);
	stream.read(reinterpret_cast<char*>(&pcxhBPP), 1);
	stream.read(reinterpret_cast<char*>(&pcxhXStart), 2);
	stream.read(reinterpret_cast<char*>(&pcxhYStart), 2);
	stream.read(reinterpret_cast<char*>(&pcxhXEnd), 2);
	stream.read(reinterpret_cast<char*>(&pcxhYEnd), 2);
	stream.read(reinterpret_cast<char*>(&pcxhDpiX), 2);
	stream.read(reinterpret_cast<char*>(&pcxhDpiY), 2);
	stream.read(reinterpret_cast<char*>(pcxhEgaPalette), 48);
	stream.read(reinterpret_cast<char*>(&pcxhReserved1), 1);
	stream.read(reinterpret_cast<char*>(&pcxhNumBitPlanes), 1);
	stream.read(reinterpret_cast<char*>(&pcxhBytesPerLine), 2);
	stream.read(reinterpret_cast<char*>(&pcxhPaletteType), 2);
	stream.read(reinterpret_cast<char*>(&pcxhHScreenSize), 2);
	stream.read(reinterpret_cast<char*>(&pcxhVScreenSize), 2);
	stream.read(reinterpret_cast<char*>(pcxhReserved2), 54);

	if (stream.fail() || pcxhId != 0x0a || pcxhVer > 5 || pcxhVer == 1 || pcxhEnc > 1 || pcxhXEnd <= pcxhXStart || pcxhYEnd <= pcxhYStart || pcxhReserved1 || *pcxhReserved2)
		throw std::runtime_error("failed to read PCX file - bogus PCXFileHeader");

	std::array<Image::Pixel_RGB_8, 256> vgaPalette;
	// a problem of PCX files: is there a VGA color palette present ?
	bool hasVGAColorPalette = (pcxhVer == 5) && (pcxhNumBitPlanes == 1) && (pcxhBPP > 4); 
	if ( hasVGAColorPalette )
	{
		// now, verify that the byte end-769 is 0xc0
		std::streampos endOfHeader = stream.tellg();
		stream.seekg(-769, std::ios_base::end);
		if (stream.fail())
			hasVGAColorPalette = false;
		else 
		{
			char VGAPaletteMarker;
			stream.read(&VGAPaletteMarker, 1);
			if (stream.fail())
				hasVGAColorPalette = false;
			else
				hasVGAColorPalette = VGAPaletteMarker == 0x0c;
		}

		if (hasVGAColorPalette)
		{
			for (int entry = 0; entry < 256; entry++)
				stream.read(reinterpret_cast<char*>(&vgaPalette[entry]), 3);
			if (stream.fail())
				throw std::runtime_error("read error occurred while reading VGA palette of file");
		}

		stream.seekg(endOfHeader, std::ios_base::beg);
		if (stream.fail())
			throw std::runtime_error("read error occurred while reading the file");
	} // end of VGA Palette handling.

	// file pointer is at the beginning of the image data ...

	// we need to calculate some values ...
	uint16_t width = pcxhXEnd - pcxhXStart + 1;
	uint16_t height = pcxhYEnd - pcxhYStart + 1;
	bool isMonochrome = (pcxhBPP == 1) && (pcxhNumBitPlanes == 1);
	bool isGrayscale = (pcxhPaletteType == 2) || isMonochrome;
	image->create(width, height, isGrayscale ? PixelMode::GRAY8 : PixelMode::RGB8, pcxhDpiX, pcxhDpiY);
	bool isRLE = (pcxhEnc == 1);


	// rle stuff ...
	uint8_t runCount;
	uint8_t runValue;
	// current position in the image
	size_t plane=0,x=0,y=0;
	int bit = 0;
	uint8_t* p; // current byte value -> point to x,y,iPlane in the image

	do
	{
		bool readOk = true;
		try {
			readNextPixel(stream, isRLE, runValue, runCount);
		}
		catch ( std::runtime_error e )
		{
			readOk = false;
		}
		for (int iRun = 0; iRun < runCount && y < height; iRun++)
		{
			do {
				uint8_t byteValue = 0;
				switch (pcxhBPP)
				{
				case 8: byteValue = runValue; break;
				case 4: byteValue = ((runValue >> (4-bit)) & 0xf); bit += 4; break;
				case 2: byteValue = ((runValue >> (6-bit)) & 0x3); bit += 2; break;
				case 1: byteValue = ((runValue >> (7-bit)) & 0x1); bit += 1; break;
				}
				if (bit == 8) bit = 0;

				if (x >= width) {
					if (static_cast<int>(x) < ((pcxhBytesPerLine * (8 / pcxhBPP))))
					{
						x++; // this is a "padding" byte/bit, just ignore it!
						continue;
					}
					else
					{
						// next plane
						x = 0;
						if (++plane >= pcxhNumBitPlanes)
						{
							// next line
							y++;
							plane = 0;
						}
					}
				}
				if (y == height)
					break; // we are done!
				if (!readOk)
					throw std::runtime_error("Premature end of PCX file");

				switch (pcxhVer)
				{
				case 5:
					switch (pcxhNumBitPlanes)
					{
					case 1:
						p = image->getPixel(y, x);
						if (!isGrayscale)
						{
							if (!hasVGAColorPalette)
							{
								if (pcxhBPP > 4)
								{
									throw std::runtime_error("PCX read failed. Image is using a VGA palette, but no VGA palette is present!");
								}
								// use EGA Palette
								*reinterpret_cast<Image::Pixel_RGB_8*>(p) = pcxhEgaPalette[byteValue];
							}
							else
								*reinterpret_cast<Image::Pixel_RGB_8*>(p) = vgaPalette[byteValue];
						}
						else // isGrayscale
						{
							switch (pcxhBPP)
							{
							case 8: *p = byteValue; break;
							case 4: *p = byteValue * 255 / 3;
							case 2: *p = byteValue * 255 / 15;
							case 1: *p = byteValue ? 255 : 0; break; // monchrome ...
							}
						}
						x++;
						break;

					case 3:
						p = image->getPixel(y, x) + plane;
						switch (pcxhBPP)
						{
						case 8: *p = byteValue; break;
						case 4: *p = byteValue * 255 / 3;
						case 2: *p = byteValue * 255 / 15;
						case 1: *p = byteValue ? 255 : 0; break;
						}
						x++;
						break;

					default:
						throw std::runtime_error("unsupported NumBitPlanes in PCX Ver 5");
					}
					break;

				default:
					throw std::runtime_error("unsupported PCX Version " + std::to_string(pcxhVer));
					break;
				}
			} while (bit != 0);
		}
	} while (y<height); // for each pixel in the pcx data
	onSubImageRead(1);	// we have read image #1
	onSubImageRead(0);	// we are done!

}

void ImageCoderPcx::write(const std::string & filename)
{
	throw std::runtime_error("writing PCX files not implemented");
}

void ImageCoderPcx::write(std::ostream & stream)
{
	throw std::runtime_error("writing PCX files not implemented");
}

void ImageCoderPcx::readNextPixel(std::istream & stream, bool isRLE, uint8_t & runValue, uint8_t & runCount)
{
	char c=0;
	stream.read(&c, 1);
	if (stream.fail())
		throw std::runtime_error("Failed to read a byte from PCX file" );
	if (isRLE)
	{
		if ((c & 0xC0) == 0xC0) /* 2-byte code */
		{
			runCount = c & 0x3F;              /* Get run count */
			stream.read(reinterpret_cast<char*>(&runValue), 1);            /* Get pixel value */
			if (stream.fail())
				throw std::runtime_error("Failed to read a byte from PCX file" );
		}
		else                                   /* 1-byte code */
		{
			runCount = 1;                     /* Run count is one */
			runValue = c;                     /* Pixel value */
		}
	}
	else
	{
		// todo:: other than 8 bits per pixel ! 
		stream.read(reinterpret_cast<char*>(&runValue), 1);
		if (stream.fail())
			throw std::runtime_error("Failed to read a byte from PCX file" );
	}
}
