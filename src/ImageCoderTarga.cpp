/* ImageCoderTarga.cpp
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
#include "ImageCoderTarga.h"

#define TGA_TRUECOLOR_24 (3)
#define TGA_TRUECOLOR_32 (4)


ImageCoderTarga::ImageCoderTarga(Image* img) 
	: ImageCoder(IE_TGA, img)
{
}

ImageCoderTarga::~ImageCoderTarga()
{
}

bool ImageCoderTarga::canDecode()
{
	return true;
}
bool ImageCoderTarga::canEncode()
{
	return false;
}
bool ImageCoderTarga::canEncode( PixelMode pixelMode )
{
	return false;
}

#define TGA_IMG_TYPE_NOIMAGE       (0)
#define TGA_IMG_TYPE_COLORMAP	   (1)
#define TGA_IMG_TYPE_RGB           (2)
#define TGA_IMG_TYPE_GRAY          (3)
#define TGA_IMG_TYPE_COLORMAP_RLE  (TGA_IMG_TYPE_COLORMAP|0x8)
#define TGA_IMG_TYPE_RGB_RLE       (TGA_IMG_TYPE_RGB|0x8)
#define TGA_IMG_TYPE_GRAY_RLE      (TGA_IMG_TYPE_GRAY|0x8)

#pragma pack(push,1)
struct TGA_HDR {
	uint8_t idLen;				// length of the image_id string; 0:no image id
	uint8_t cmapType;			// 0: no palette; 1: palette
	uint8_t imageType;			// SEE TGA_IMG_FMT_*
	uint16_t cmapFirstIndex;	// index of first color map entry that is included in the file
	uint16_t cmapLength;		// length of the color map - in number of entries
	uint8_t  cmapEntrySize;		// size of one entry in the color map - in bits (typical: 15, 16, 24, 32)
	uint16_t xOrigin;			// absolute coordinate of lower-left corner for displays where origin is at the lower left
	uint16_t yOrigin;			// absolute coordinate of lower-left corner for displays where origin is at the lower left
	uint16_t width;				// pixels
	uint16_t height;			// pixels
	uint8_t  pixelDepth;		// bits per pixel...
	uint8_t  imageDescription;	// 0x0f: alpha, 0x30: direction
};
#pragma pack(pop)

/* TARGA header is not clearly identfied. We do some heuristic checks. */
bool ImageCoderTarga::mightBeTargaHeader(uint8_t* byte18)
{
	struct TGA_HDR* h = reinterpret_cast<struct TGA_HDR*>(byte18);
	// valid cmapType ?
	switch (h->cmapType) {
	case 0: // no palette
		// according to the spec, cmapFirstIndex, cmapLength and cmapEntrySize "should" be 0 (unfortunately, this is not a "must", but we require it)
//		if (h->cmapFirstIndex || h->cmapLength /*|| h->cmapEntrySize*/)
//			return false;
		break;
	case 1:
		break;
	default:
		return false; // in theorie, other values are allowed, but then we can't read the file anyways.
	}
	// valid imageType (note that there are others in exotic versions, like huffmann encoding, that we don't support)
	switch (h->imageType) {
	case TGA_IMG_TYPE_NOIMAGE:	
		break; // we can't read it, but it still is a valid targa file ... we will fail during read, though.
	case TGA_IMG_TYPE_COLORMAP: 
	case TGA_IMG_TYPE_COLORMAP_RLE:
		if (h->cmapType == 0) // color map type images require a color map !
			return false;
		break;
	case TGA_IMG_TYPE_RGB:
	case TGA_IMG_TYPE_RGB_RLE:
		//		if (h->cmapType != 0) // some RGB images actually DO have a color map, unfortunately...
//			return false;
		break;
	case TGA_IMG_TYPE_GRAY:
	case TGA_IMG_TYPE_GRAY_RLE:
		if (h->cmapType != 0) // gray images might not have a color map!
			return false;
		break;
	default:
		return false;
	}

	
	if (h->cmapType != 0) {
		// color palette indicated, so bytes shall not be all null
		if (h->cmapEntrySize == 0)
			return false;
		if (h->cmapLength == 0)
			return false;
		if (h->cmapLength > 8192) // not sure where I have this from ... It's not from the official spec. 
		                          // Wikipedia actually says, the palette might be up to 8192 BYTES (which would mean max of 2048 entries @ 32 bit length); 
		                          // I know that 4096@8bit palettes are allowed. So let's stay with the 8192.
			return false;
		if (h->cmapFirstIndex >= h->cmapLength) // well, the first entry certainly can't be out of the palette ...
			return false;
	}

	switch (h->cmapEntrySize) {
	case 0:
	case 15:
	case 16:
	case 24:
	case 32:
		break;
	default:
		return false;
	}

	if (h->imageDescription & 0xc) // bits 7 and 6 must be 0 as of tga 2.0 
	{
		return false;
	}
	return true;
}

void ImageCoderTarga::read(const std::string& filename)
{
	std::ifstream file(filename.c_str(), std::ios_base::binary);
	if (file.fail())
		throw std::runtime_error("failed to open DPEG file " + filename);
	read(file);
}

void ImageCoderTarga::read( std::istream & stream ) 
{

	//
	// READ TGA HEADER, which is 18 bytes long.
	//
	
	struct TGA_HDR header;

	// header is actually 18 bytes... struct alignment issues, can't read header directly ...
	stream.read(reinterpret_cast<char*>(&header.idLen), 1);
	stream.read(reinterpret_cast<char*>(&header.cmapType), 1);
	stream.read(reinterpret_cast<char*>(&header.imageType), 1);
	stream.read(reinterpret_cast<char*>(&header.cmapFirstIndex), 2);
	stream.read(reinterpret_cast<char*>(&header.cmapLength), 2);
	stream.read(reinterpret_cast<char*>(&header.cmapEntrySize), 1);
	stream.read(reinterpret_cast<char*>(&header.xOrigin), 2);
	stream.read(reinterpret_cast<char*>(&header.yOrigin), 2);
	stream.read(reinterpret_cast<char*>(&header.width), 2);
	stream.read(reinterpret_cast<char*>(&header.height), 2);
	stream.read(reinterpret_cast<char*>(&header.pixelDepth), 1);
	stream.read(reinterpret_cast<char*>(&header.imageDescription), 1);
	if (stream.fail())
		throw std::runtime_error("Failed to read TGA header");

	//
	// READ ImageID if present
	//
	std::string imageId;
	if (header.idLen > 0)
	{
		char buffer[255];
		stream.read(buffer, header.idLen);
		if (stream.fail())
			throw std::runtime_error("Failure reading TGA imageId");
		imageId.assign(buffer, header.idLen);
	}

	//
	// INTERPRET IMAGE DESCRIPTION
	// 0-3: alphaBits (value = 0..15)
	// 4: 0=first is leftmost pixel; 1=first is rightmost pixel
	// 5: 0=first is bottommost pixel; 1=first is topmost pixeö

	// alpha ?
	uint8_t alphaBits = header.imageDescription & 0x0f;	// 0-3 ...
	bool hasAlpha = alphaBits != 0;
	bool mirrorX = (header.imageDescription & 0x10) != 0;
	bool mirrorY = (header.imageDescription & 0x20) == 0;

	if (header.width == 0 || header.height == 0 )
		throw std::runtime_error("TGA: invalid pixel size");

	// check image type / color map combination ...
	bool isGray = false;
	switch (header.imageType)
	{
	case TGA_IMG_TYPE_NOIMAGE:
		throw std::runtime_error("TGA: no image data");

	case TGA_IMG_TYPE_RGB:
	case TGA_IMG_TYPE_RGB_RLE:
		if (header.cmapType != 0)
		{
			// theoretically, this is not allowed, but there might be some TGA files encoded like that... 
			//	throw std::runtime_error("TGA: color map for RGB image not allowed");
		}
		break;

	case TGA_IMG_TYPE_GRAY:
	case TGA_IMG_TYPE_GRAY_RLE:
		if (header.cmapType != 0)
		{
			throw std::runtime_error("TGA: color map for GRAY image not allowed");
		}
		isGray = true;
		break;

	case TGA_IMG_TYPE_COLORMAP:
	case TGA_IMG_TYPE_COLORMAP_RLE:
		if (header.cmapType == 0)
		{
			throw std::runtime_error("TGA: color map type missing for COLORMAP image type");
		}
		break;

	default:
		throw std::runtime_error("TGA: Invalid Image type");
	}
	// at this type, header.cmapType != 0 tells us, we have a color map...

	// Color map ...
	std::vector<Image::Pixel_ARGB_8> colorMap; // ARGB
	uint8_t buffer[4];
	if (header.cmapType)
	{
		size_t fileColorMapBytesPerEntry;	// size in file
		switch (header.cmapEntrySize) {
		case 15: // -BBBBBGGGGGRRRRR
			fileColorMapBytesPerEntry = 2;
			break;
		case 16: // OBBBBBGGGGGRRRRR
			fileColorMapBytesPerEntry = 2;
//			hasAlpha = true; // alphaBits not set for colorMap! Hmmmm ... not sure exactly what that overlay bit does, but it deosn't seem to be real alpha ...
			break;
		case 24: //BBBBBBBBGGGGGGGGRRRRRRRR
			fileColorMapBytesPerEntry = 3;
			break;
		case 32: //AAAAAAAABBBBBBBBGGGGGGGGRRRRRRRR
			fileColorMapBytesPerEntry = 4;
			hasAlpha = true; // alphaBits not set for colorMap!
			break;
		default:
			throw std::runtime_error("TGA: unsupported Colormap Entry Size!");
		}

		colorMap.resize(header.cmapLength);
		uint8_t r, g, b, a;
		a = 0xff;
		for (size_t iEntry = 0; iEntry < header.cmapLength; iEntry++)
		{
			// not sure how to handle header.cmapFirstIndex > 0, is that right?
			if (iEntry < header.cmapFirstIndex)
			{
				b = g = r = a = 0xff; // white non transparent
			}
			else
			{
				stream.read(reinterpret_cast<char*>(buffer), fileColorMapBytesPerEntry);

				switch (header.cmapEntrySize)
				{
				case 16:
					a = (buffer[1] & 0x80) ? 0xff : 0; // this way arround ? or the other way arround ?
				case 15: // -RRRRRGG GGGBBBBB
					r = scale5To8bit((buffer[1] & 0x7c) >> 2); // -RRRRRgg
					g = scale5To8bit(((buffer[1] & 0x03) << 3) | ((buffer[0] & 0xe0) >> 5) ); // -rrrrrGG | GGGbbbbb
					b = scale5To8bit(buffer[0] & 0x1f); // gggBBBBB
					break;
				case 24:
					r = buffer[2];
					g = buffer[1];
					b = buffer[0];
					break;
				case 32:
					a = buffer[3];	
					r = buffer[2];
					g = buffer[1];
					b = buffer[0];
					if (a != 0 && a != 255)
					{
						// ALpha is premultiplied... un-pre-multiply it...
						r = (r * a / 255);
						g = (g * a / 255);
						b = (b * a / 255);
					}
					break;
				}
			}
			colorMap[iEntry].alpha = a;
			colorMap[iEntry].r = r;
			colorMap[iEntry].g = g;
			colorMap[iEntry].b = b;
		}
	}

	size_t bytesPerPixel = header.pixelDepth >> 3;
	if (header.pixelDepth & 0x07)
		bytesPerPixel += 1;

	// compute the true number of bits per pixel
//	true_bits_per_pixel = cmap_type ? cmap_entry_size : img_spec_pix_depth;


	PixelMode pm = PixelMode::create(false, !isGray, isGray, false, hasAlpha, 8);
	image->create(header.width, header.height, pm);

	uint32_t value;
	bool rle = false;
	size_t row2;
	// reset any rle buffers ...
	rleLastVal = 0;
	rleRepeatsLeft = 0;
	rleRawLeft = 0;
	switch (header.imageType)
	{
	case TGA_IMG_TYPE_COLORMAP_RLE:
		rle = true;
	case TGA_IMG_TYPE_COLORMAP:
		// value is index ...
		for (size_t row = 0; row < header.height; row++)
		{
			row2 = (mirrorY) ? (header.height - row - 1) : row;
			uint8_t* p = mirrorX ? image->getPixel(row2, header.width-1) : image->getLine(row2);
			for (size_t col = 0; col < header.width; col++)
			{
				value = readNextPixel(stream, bytesPerPixel, rle);
				if (value >= colorMap.size())
					throw std::runtime_error("pixel value out of color map range!");
				if (hasAlpha)
				{
					int alpha = p[0] = colorMap[value].alpha;
					int r, g, b; // r,g,b are pre-multiplied, we need to un-premultiply....
					r = colorMap[value].r;
					g = colorMap[value].g;
					b = colorMap[value].b;
					if (alpha != 0 && alpha != 255)
					{
						r = (r * alpha / 255);
						g = (g * alpha / 255);
						b = (b * alpha / 255);
					}
					p[1] = r & 0xff;
					p[2] = g & 0xff;
					p[3] = b & 0xff;
					if (mirrorX)
						p -= 4;
					else
						p += 4;

				}
				else
				{
					*p++ = colorMap[value].r;
					*p++ = colorMap[value].g;
					*p++ = colorMap[value].b;
					if (mirrorX)
						p -= 6;
				}
			}
		}
		break;

	case TGA_IMG_TYPE_RGB_RLE:
		rle = true;
	case TGA_IMG_TYPE_RGB:
		// value is color (as ARGB)
		for (size_t row = 0; row < header.height; row++)
		{
			int a = 0xff;
			int r, g, b; 
			row2 = (mirrorY) ? (header.height - row - 1) : row;
			uint8_t* p = mirrorX ? image->getPixel(row2, header.width - 1) : image->getLine(row2);
			for (size_t col = 0; col < header.width; col++)
			{
				value = readNextPixel(stream, bytesPerPixel, rle);
				switch (header.pixelDepth) {
				case 32:
					a = (value & 0xff000000) >> 24;
					// no break
				case 24:
					r = (value & 0xff0000) >> 16;
					g = (value & 0xff00) >> 8;
					b = (value & 0xff);
					break;
				case 16: // ARRRRRGG GGGBBBBB
					a = (value & 0x8000) ? 0xff : 0;
					// no break
				case 15:
					r = scale5To8bit( (value & 0x7c00) >> 10 ); 
					g = scale5To8bit( (value & 0x03e0) >> 5  ); 
					b = scale5To8bit( (value & 0x001f)       ); 
					break;
				}

				if (hasAlpha)
				{
					p[0] = a;

					// r,g,b are pre-multiplied, we need to un-premultiply....
					if (a != 0 && a != 255)
					{
						r = (r * a / 255);
						g = (g * a / 255);
						b = (b * a / 255);
					}

					p[1] = r & 0xff;
					p[2] = g & 0xff;
					p[3] = b & 0xff;
					if (mirrorX)
						p -= 4;
					else
						p += 4;
				}
				else
				{
					p[0] = r;
					p[1] = g;
					p[2] = b;
					if (mirrorX)
						p -= 3;
					else
						p += 3;
				}
			}
		}
		break;

	case TGA_IMG_TYPE_GRAY_RLE:
		rle = true;
	case TGA_IMG_TYPE_GRAY:
		// value is color (grayvalue)
		for (size_t row = 0; row < header.height; row++)
		{
			row2 = (mirrorY) ? (header.height - row - 1) : row;
			uint8_t* p = mirrorX ? image->getPixel(row2, header.width - 1) : image->getLine(row2);
			for (size_t col = 0; col < header.width; col++)
			{
				value = readNextPixel(stream, bytesPerPixel, rle);
				if (hasAlpha)
				{
					*p++ = (value & 0xff00) >> 8;
					*p++ = value & 0xff;
					if (mirrorX)
						p-=4;
				}
				else
				{
					*p = value & 0xff;
					if (mirrorX)
						p--;
					else
						p++;
				}
			}
		}
		break;
	}

	onSubImageRead(1);	// we have read image #1
	onSubImageRead(0);	// we are done!

}

void ImageCoderTarga::write(const std::string& filename)
{
	throw std::runtime_error("writing Targa (TGA) files not implemented");
}

void ImageCoderTarga::write(std::ostream& stream)
{
	throw std::runtime_error("writing Targa (TGA) files not implemented");
}

uint32_t ImageCoderTarga::readNextPixel(std::istream & stream, size_t bytesPerPixel, bool rle)
{
	if (rle)
	{
		if (rleRepeatsLeft > 0)
		{
			rleRepeatsLeft--;
			return rleLastVal;
		}
		if (rleRawLeft == 0)
		{
			// read rle/raw packt
			uint8_t rleInfo;
			stream.read(reinterpret_cast<char*>(&rleInfo), 1);
			if ( (rleInfo & 0x80) == 0x80)
			{
				// this is a RLE packet, repeat next pixel x number of times
				rleRepeatsLeft = (rleInfo & 0x7f);
			}
			else
			{
				// this is a RAW packet, the next x number of pixels are raw
				rleRawLeft = (rleInfo & 0x7f);
			}
		}
		else
		{
			rleRawLeft--;
		}
	}
	rleLastVal = 0;
	char* buffer = reinterpret_cast<char*>(&rleLastVal);
	stream.read(buffer, bytesPerPixel);
	if (stream.fail())
		throw new std::runtime_error("TGA: EOF while reading image data!");
	
	return rleLastVal;
}

int ImageCoderTarga::scale5To8bit(int _5bitVal) {
	static const int val8[32] = {
		0, 8, 16, 25, 33, 41, 49, 58,
		66, 74, 82, 90, 99, 107, 115, 123,
		132, 140, 148, 156, 165, 173, 181, 189,
		197, 206, 214, 222, 230, 239, 247, 255 };
	return val8[_5bitVal & 0x1f];
}