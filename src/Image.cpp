/* Image.cpp
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
#include "Image.h"
#include <fstream>
#include "ImageCoder.h"

// Statics
const std::string Image::ColorModel_Gray{ "GRAY" };
const std::string Image::ColorModel_RGB{ "RGB" };
const std::string Image::ColorModel_CMYK{ "CMYK" };
const std::string Image::ColorModel_Lab( "Lab" );


static inline uint16_t trans8to16(uint8_t u8) { return static_cast<uint16_t>((u8 << 8) | u8); }
static inline float    trans8to32(uint8_t u8) { return u8 / 255.0f; }
static inline uint8_t  trans16to8(uint16_t u16) { return static_cast<uint8_t>(u16 >> 8); }
static inline float    trans16to32(uint16_t u16) { return u16 / 65535.0f; }
static inline uint8_t  trans32to8(float f32) { return static_cast<uint8_t>(f32 * 255.0f + 0.5f); }
static inline uint16_t trans32to16(float f32) { return static_cast<uint16_t>(f32 * 65535.0f + 0.5f); }

//class colorModel;
Image::Image() : pixelMode(PixelMode::ARGB8)
{
	static_assert(sizeof(float) == 4, "Unexpected size of float data type.");
}


Image::~Image()
{
	destroy();
}
void Image::destroy()
{
	header.width = 0;
	header.height = 0;
	header.imageSize = 0;
	pixels.reset();
	iccProfile.clear();
}

void Image::create(size_t width, size_t height, PixelMode pm, double dpiX, double dpiY)
{
	destroy();
	pixelMode = pm;
	header.nSize      = sizeof(Header); // sizeof struct
	header.ID         = 0; // Version, always equals 0
	header.dataOrder  = 0; // data order pixel (1: planes)
	header.origin     = 0; // top left (1: bottom left);
	header.pixelAlign = 1;
	header.lineAlign  = sizeof(long);
	this->dpiX = dpiX;
	this->dpiY = dpiY;
	switch (pixelMode)
	{
	case PixelMode::GRAY8:
		header.nChannels = 1;
		header.depth = 8;
		header.alphaChannel = -1;
		header.colorModel = ColorModel_Gray;
		header.channelSeq = "g";
		header.bytesPerPixel = 1;
		break;
	case PixelMode::GRAY16:
		header.nChannels = 1;
		header.depth = 16;
		header.alphaChannel = -1;
		header.colorModel = ColorModel_Gray;
		header.channelSeq = "g";
		header.bytesPerPixel = 2;
		break;
	case PixelMode::GRAY32:
		header.nChannels = 1;
		header.depth = 32;
		header.alphaChannel = -1;
		header.colorModel = ColorModel_Gray;
		header.channelSeq = "g";
		header.bytesPerPixel = 4;
		break;
	case PixelMode::AGRAY8:
		header.nChannels = 2;
		header.depth = 8;
		header.alphaChannel = 0;
		header.colorModel = ColorModel_Gray;
		header.channelSeq = "Ag";
		header.bytesPerPixel = 2;
		break;
	case PixelMode::AGRAY16:
		header.nChannels = 2;
		header.depth = 16;
		header.alphaChannel = 0;
		header.colorModel = ColorModel_Gray;
		header.channelSeq = "Ag";
		header.bytesPerPixel = 4;
		break;
	case PixelMode::AGRAY32:
		header.nChannels = 2;
		header.depth = 32;
		header.alphaChannel = 0;
		header.colorModel = ColorModel_Gray;
		header.channelSeq = "Ag";
		header.bytesPerPixel = 8;
		break;
	case PixelMode::RGB8:
		header.nChannels = 3;
		header.depth = 8;
		header.alphaChannel = -1;
		header.colorModel = ColorModel_RGB;
		header.channelSeq = "RGB";
		header.bytesPerPixel = 3;
		break;
	case PixelMode::RGB16:
		header.nChannels = 3;
		header.depth = 16;
		header.alphaChannel = -1;
		header.colorModel = ColorModel_RGB;
		header.channelSeq = "RGB";
		header.bytesPerPixel = 6;
		break;
	case PixelMode::RGB32:
		header.nChannels = 3;
		header.depth = 32;
		header.alphaChannel = -1;
		header.colorModel = ColorModel_RGB;
		header.channelSeq = "RGB";
		header.bytesPerPixel = 12;
		break;
	case PixelMode::ARGB8:
		header.nChannels = 4;
		header.depth = 8;
		header.alphaChannel = 0;
		header.colorModel = ColorModel_RGB;
		header.channelSeq = "ARGB";
		header.bytesPerPixel = 4;
		break;
	case PixelMode::ARGB16:
		header.nChannels = 4;
		header.depth = 16;
		header.alphaChannel = 0;
		header.colorModel = ColorModel_RGB;
		header.channelSeq = "ARGB";
		header.bytesPerPixel = 8;
		break;
	case PixelMode::ARGB32:
		header.nChannels = 4;
		header.depth = 32;
		header.alphaChannel = 0;
		header.colorModel = ColorModel_RGB;
		header.channelSeq = "ARGB";
		header.bytesPerPixel = 16;
		break;
	case PixelMode::CMYK8:
		header.nChannels = 4;
		header.depth = 8;
		header.alphaChannel = -1;
		header.colorModel = ColorModel_CMYK;
		header.channelSeq = "CMYK";
		header.bytesPerPixel = 4;
		break;
	case PixelMode::CMYK16:
		header.nChannels = 4;
		header.depth = 16;
		header.alphaChannel = -1;
		header.colorModel = ColorModel_CMYK;
		header.channelSeq = "CMYK";
		header.bytesPerPixel = 8;
		break;
	case PixelMode::CMYK32:
		header.nChannels = 4;
		header.depth = 32;
		header.alphaChannel = -1;
		header.colorModel = ColorModel_CMYK;
		header.channelSeq = "CMYK";
		header.bytesPerPixel = 16;
		break;
	case PixelMode::CMYKA8:
		header.nChannels = 5;
		header.depth = 8;
		header.alphaChannel = 4;
		header.colorModel = ColorModel_CMYK;
		header.channelSeq = "CMYKA";
		header.bytesPerPixel = 5;
		break;
	case PixelMode::CMYKA16:
		header.nChannels = 5;
		header.depth = 16;
		header.alphaChannel = 4;
		header.colorModel = ColorModel_CMYK;
		header.channelSeq = "CMYKA";
		header.bytesPerPixel = 10;
		break;
	case PixelMode::CMYKA32:
		header.nChannels = 5;
		header.depth = 32;
		header.alphaChannel = 4;
		header.colorModel = ColorModel_CMYK;
		header.channelSeq = "CMYKA";
		header.bytesPerPixel = 20;
		break;
	case PixelMode::LAB32:
		header.nChannels = 3;
		header.depth = 32;
		header.alphaChannel = -1;
		header.colorModel = ColorModel_Lab;
		header.channelSeq = "Lab";
		header.bytesPerPixel = 12;
		break;
	case PixelMode::LABA32:
		header.nChannels = 4;
		header.depth = 32;
		header.alphaChannel = 0;
		header.colorModel = ColorModel_Lab;
		header.channelSeq = "LabA";
		header.bytesPerPixel = 16;
		break;
	}

	// calculate bytesPerPixel and bytesPerLine based on alignment!
	int overhang;
	overhang = header.bytesPerPixel % header.pixelAlign;
	if (overhang > 0) header.bytesPerPixel += header.pixelAlign - overhang;

	header.width = width;
	header.height = height;
	// calculate width of line in bytes (inlcuding padding)
	header.widthStep = width * header.bytesPerPixel;
	overhang = header.widthStep % header.lineAlign;
	if (overhang > 0) header.widthStep += header.lineAlign - overhang;

	header.imageSize = header.widthStep * header.height;
	pixels.reset( new uint8_t[header.imageSize] );
}
void Image::createFrom(const Image* other)
{
	create( other->header.width, other->header.height, other->pixelMode, other->dpiX, other->dpiY );
}
void Image::copy(const Image* other)
{
	create( other->header.width, other->header.height, other->pixelMode, other->dpiX, other->dpiY );
	std::copy( other->pixels.get(), other->pixels.get() + other->header.imageSize, this->pixels.get() );
	iccProfile = other->iccProfile;
}
void Image::copyAlphaFrom(const Image* other)
{
	if ( other->header.width != header.width || other->header.height != header.height )
		throw std::runtime_error( "copyAlphaFrom failed, src image has different size" );
	if (other->header.depth != 8 && other->header.depth != 16)
		throw std::runtime_error("copyAlphaFrom failed, src image has unsupported depth");
	if (header.depth != 8 && header.depth != 16)
		throw std::runtime_error("copyAlphaFrom failed, dest image has unsupported depth");
	if (!other->hasAlpha())
		throw std::runtime_error( "copyAlphaFrom failed, src image has no Alpha Channel" );
	if (!this->hasAlpha()  )
		throw std::runtime_error( "copyAlphaFrom failed, destination image has no Alpha Channel" );
	for (size_t iLine = 0; iLine < header.height; iLine++)
	{
		for (size_t iRow = 0; iRow < header.width; iRow++)
		{
			const uint8_t* p = other->getPixelConst(iLine, iRow);
			switch (header.depth)
			{
			case 8:
				switch (other->header.depth)
				{
				case 8:
					reinterpret_cast<uint8_t*>(this->getPixel(iLine, iRow))[header.alphaChannel] = p[other->header.alphaChannel];
					break;
				case 16:
					reinterpret_cast<uint8_t*>(this->getPixel(iLine, iRow))[header.alphaChannel] = trans16to8(reinterpret_cast<const uint16_t*>(p)[other->header.alphaChannel]);
					break;
				case 32:
					reinterpret_cast<uint8_t*>(this->getPixel(iLine, iRow))[header.alphaChannel] = trans32to8(reinterpret_cast<const float*>(p)[other->header.alphaChannel]);
					break;
				}
				break;
			case 16:
				switch (other->header.depth)
				{
				case 8:
					reinterpret_cast<uint16_t*>(this->getPixel(iLine, iRow))[header.alphaChannel] = trans8to16(p[other->header.alphaChannel]);
					break;
				case 16:
					reinterpret_cast<uint16_t*>(this->getPixel(iLine, iRow))[header.alphaChannel] = reinterpret_cast<const uint16_t*>(p)[other->header.alphaChannel];
					break;
				case 32:
					reinterpret_cast<uint16_t*>(this->getPixel(iLine, iRow))[header.alphaChannel] = trans32to16(reinterpret_cast<const float*>(p)[other->header.alphaChannel]);
					break;
				}
				break;
			case 32:
				switch (other->header.depth)
				{
				case 8:
					reinterpret_cast<float*>(this->getPixel(iLine, iRow))[header.alphaChannel] = trans8to32(p[other->header.alphaChannel]);
					break;
				case 16:
					reinterpret_cast<float*>(this->getPixel(iLine, iRow))[header.alphaChannel] = trans16to32(reinterpret_cast<const uint16_t*>(p)[other->header.alphaChannel]);
					break;
				case 32:
					reinterpret_cast<float*>(this->getPixel(iLine, iRow))[header.alphaChannel] = reinterpret_cast<const float*>(p)[other->header.alphaChannel];
					break;
				}
				break;
			}
		}
	}
}
std::unique_ptr<Image> Image::clone() const
{
	std::unique_ptr<Image> newImg( new Image() );
	newImg->copy( this );
	return newImg;
}

uint8_t* Image::getLine(size_t line)
{
	// UNCHECKED ! Make sure line < height !;
	return pixels.get() + (header.widthStep * line);
}

const uint8_t* Image::getLineConst(size_t line) const
{
	// UNCHECKED ! Make sure line < height !;
	return pixels.get() + (header.widthStep * line);
}

uint8_t* Image::getPixel(size_t line, size_t column)
{
	return getPixel( getLine(line), column );
}

const uint8_t* Image::getPixelConst(size_t line, size_t column) const
{
	return getPixelConst(getLineConst(line), column);
}

uint8_t* Image::getPixel(void* line, size_t column)
{
	// UNCHECKED ! column < header.width !
	return reinterpret_cast<uint8_t*>(line) + (header.bytesPerPixel * column);
}

const uint8_t* Image::getPixelConst(const void* line, size_t column) const
{
	return reinterpret_cast<const uint8_t*>(line)+(header.bytesPerPixel * column);
}

void Image::read(const std::string & filename, size_t subImage/*=1*/, ImageCoderProperties * props/*=nullptr*/)
{
	subImageToRead = subImage;
	subImageRead = 0;
	
	std::ifstream file(filename.c_str(), std::ios_base::binary);
	if (file.fail())
		throw std::runtime_error("failed to open file: " + filename);

	// determine file type ...
	ImageEncoding imgenc = ImageCoder::determineImageType(file);

	file.close();

	// create our own, local properties if none given as parameter
	std::unique_ptr<ImageCoderProperties> defaultProps;
	if ( !props )
	{
		defaultProps.reset(new ImageCoderProperties);
		props = defaultProps.get();
	}
	
	std::unique_ptr<ImageCoder> coder( ImageCoder::createCoder(imgenc,this,props) );
	if ( !coder )
		throw std::runtime_error( "No encoder for this image type!" );

	if (! coder->canDecode() )
		throw std::runtime_error("selected ImageCoder can't decode.");

	coder->read(filename);
	
	if ( subImageRead != subImage )
		throw std::runtime_error("sub image (page) " + std::to_string(subImage) + "not found in file.");
}

void Image::read(std::istream & stream, const std::string imageName/*=""*/, size_t subImage /*= 1*/, ImageCoderProperties* props /*= nullptr*/)
{
	subImageToRead = subImage;
	subImageRead = 0;

	// determine file type ... resets stream pointer to original pos afterwards
	ImageEncoding imgenc = ImageCoder::determineImageType(stream);

	// create our own, local properties if none given as parameter
	std::unique_ptr<ImageCoderProperties> defaultProps;
	if (!props)
	{
		defaultProps.reset(new ImageCoderProperties);
		props = defaultProps.get();
	}

	std::unique_ptr<ImageCoder> coder(ImageCoder::createCoder(imgenc, this, props));
	if (!coder)
		throw std::runtime_error("No encoder for this image type!");

	if (!coder->canDecode())
		throw std::runtime_error("selected ImageCoder can't decode.");

	coder->read(stream);

	if (subImageRead != subImage)
		throw std::runtime_error("sub image (page) " + std::to_string(subImage) + "not found in file.");

}

void Image::read(const void* imageBuffer, size_t imageBufferSize, const std::string imageName /*= ""*/, size_t subImage /*= 1*/, ImageCoderProperties* props /*= nullptr*/)
{
	// for now we copy, the better solution would be to override streambuf and istream, that avoids a copy. But needs lot of testing.
	std::istringstream ss(std::string(reinterpret_cast<const char*>(imageBuffer), imageBufferSize));
	read(ss, imageName, subImage, props);
}


void Image::write(const std::string & filename, ImageEncoding imgenc, ImageCoderProperties* props/*=nullptr*/ )
{
	// create our own, local properties if none given as parameter
	std::unique_ptr<ImageCoderProperties> defaultProps;
	if ( !props )
	{
		defaultProps.reset(new ImageCoderProperties);
		props = defaultProps.get();
	}
	
	std::unique_ptr<ImageCoder> coder( ImageCoder::createCoder(imgenc,this,props) );
	if ( !coder )
		throw std::runtime_error( "No encoder for this image type!" );

	if (! coder->canEncode() )
		throw std::runtime_error("selected ImageCoder can't encode.");
	if (! coder->canEncode(pixelMode) )
		throw std::runtime_error("selected ImageCoder can't encode pixel mode of current image.");

	coder->write(filename );
}




PixelMode Image::getPixelMode() const
{
	return pixelMode;
}

uint32_t Image::getWidth() const
{
	return static_cast<uint32_t>(header.width);
}

uint32_t Image::getHeight() const
{
	return static_cast<uint32_t>(header.height);
}

uint32_t Image::getNumPixels() const
{
	return getWidth() * getHeight();
}


uint32_t Image::getStride() const
{
	return static_cast<uint32_t>(header.widthStep);
}

uint32_t Image::getChannelSize() const
{
	return static_cast<uint32_t>(header.depth/8);
}

uint32_t Image::getPixelSize() const
{
	return static_cast<uint32_t>(header.bytesPerPixel);
}

double Image::getResolutionX() const
{
	return dpiX;
}

double Image::getResolutionY() const
{
	return dpiY;
}

void Image::setResolution(double dpiX, double dpiY)
{
	this->dpiX = dpiX;
	this->dpiY = dpiY;
}


bool Image::isCMYK() const
{
	return pixelMode.isCMYK();
}

bool Image::isRGB() const
{
	return pixelMode.isRGB();
}

bool Image::isGray() const
{
	return pixelMode.isGray();
}

bool Image::isLab() const
{
	return pixelMode.isLab();
}

bool Image::hasAlpha() const
{
	return pixelMode.hasAlpha();
}

bool Image::is32bpc() const
{
	return pixelMode.getBitsPerChannel() == 32;
}

bool Image::is16bpc() const
{
	return pixelMode.getBitsPerChannel()==16;
}

bool Image::is8bpc() const
{
	return pixelMode.getBitsPerChannel()==8;
}

/* Fill image with given "Color" */
void Image::fill(const void* pixel, size_t x/*=0*/, size_t y/*=0*/, size_t w/*=0*/, size_t h/*=0 */)
{
	if ( w==0 ) w = header.width - x;
	if ( h==0 ) h = header.height - y;
	
	const uint8_t* q = reinterpret_cast<const uint8_t*>(pixel);
	for ( size_t iLine=y; iLine<h; iLine++ )
	{
		uint8_t* p = reinterpret_cast<uint8_t*>(getLine(iLine));
		for ( size_t iCol=x; iCol< w; iCol++ )
			for ( int iByte=0; iByte < header.bytesPerPixel; iByte++ )
				*p++ = q[iByte];
	}
}

bool Image::imageReadCallback(size_t iSubImageRead)
{
	if ( iSubImageRead == 0 )
	{
		return false;	// end of file - do not continue
	}
	else if ( iSubImageRead == subImageToRead )
	{
		subImageRead = iSubImageRead;
		return false;	// do not continue, we have all we need
	}
	return true;
}

void Image::convertTo16bpc(Image* out)
{
	if ( is16bpc() )
	{
		// already 16 bpc
		if ( out ) out->copy(this);
		return;
	}

	if (isLab())
		throw std::runtime_error("convertTo16bpc failed: not supported for L*a*b* images");

	bool inPlace = out == nullptr;
	if ( inPlace )
	{
		out = new Image();
	}
	PixelMode newMode = PixelMode::create( pixelMode.isCMYK(), pixelMode.isRGB(), pixelMode.isGray(), pixelMode.isLab(), pixelMode.hasAlpha(), 16 );
	out->create( this->header.width, this->header.height, newMode, this->dpiX, this->dpiY );
	out->iccProfile = this->iccProfile;

	for (size_t iLine = 0; iLine < header.height; iLine++)
	{
		uint16_t* p = reinterpret_cast<uint16_t*>(out->getLine(iLine));
		if (is8bpc()) {
			uint8_t* q = reinterpret_cast<uint8_t*>(this->getLine(iLine));
			for (size_t iSample = 0; iSample < header.width*header.nChannels; iSample++)
			{
				*p++ = trans8to16(*q++);
			}
		}
		else if (is32bpc()) {
			float* q = reinterpret_cast<float*>(this->getLine(iLine));
			for (size_t iSample = 0; iSample < header.width*header.nChannels; iSample++)
			{
				*p++ = trans32to16(*q++);
			}
		}
	}
	if ( inPlace )
	{
		this->createFrom(out);
		this->iccProfile = out->iccProfile;
		this->pixels.swap( out->pixels );
		delete out;
		out = nullptr;
	}
}

void Image::convertTo8bpc(Image* out)
{
	if ( is8bpc() )
	{
		// already 8 bpc
		if ( out ) out->copy(this);
		return;
	}

	if ( isLab() )
		throw std::runtime_error("convertTo8bpc failed: not supported for L*a*b* images");

	bool inPlace = out==nullptr;
	if ( inPlace )
	{
		out = new Image();
	}
	PixelMode newMode = PixelMode::create( pixelMode.isCMYK(), pixelMode.isRGB(), pixelMode.isGray(), pixelMode.isLab(), pixelMode.hasAlpha(), 8 );
	out->create( this->header.width, this->header.height, newMode, this->dpiX, this->dpiY );
	out->iccProfile = this->iccProfile;
				 
	for ( size_t iLine=0; iLine < header.height; iLine++ )
	{
		uint8_t* p = reinterpret_cast<uint8_t*>(out->getLine(iLine));
		if (is16bpc()) {
			uint16_t* q = reinterpret_cast<uint16_t*>(this->getLine(iLine));
			for (size_t iSample = 0; iSample < header.width*header.nChannels; iSample++)
			{
				*p++ = static_cast<uint8_t>((*q++) >> 8);
			}
		}
		else if (is32bpc()) {
			float* q = reinterpret_cast<float*>(this->getLine(iLine));
			for (size_t iSample = 0; iSample < header.width*header.nChannels; iSample++)
			{
				*p++ = static_cast<uint8_t>((*q++) * 255.0f + 0.5f);
			}
		}

	}
	if ( inPlace )
	{
		this->createFrom(out);
		this->iccProfile = out->iccProfile;
		this->pixels.swap( out->pixels );
		delete out;
		out = nullptr;
	}	
}

void Image::convertTo32bpc(Image* out)
{
	if (is32bpc())
	{
		// already 32 bpc
		if (out) out->copy(this);
		return;
	}

	bool inPlace = out == nullptr;
	if (inPlace)
	{
		out = new Image();
	}
	PixelMode newMode = PixelMode::create(pixelMode.isCMYK(), pixelMode.isRGB(), pixelMode.isGray(), pixelMode.isLab(), pixelMode.hasAlpha(), 32);
	out->create(this->header.width, this->header.height, newMode, this->dpiX, this->dpiY);
	out->iccProfile = this->iccProfile;

	for (size_t iLine = 0; iLine < header.height; iLine++)
	{
		float* p = reinterpret_cast<float*>(out->getLine(iLine));
		if (is8bpc()) {
			uint8_t* q = reinterpret_cast<uint8_t*>(this->getLine(iLine));
			for (size_t iSample = 0; iSample < header.width*header.nChannels; iSample++)
			{
				*p++ = static_cast<float>(*q++) / 255.0f;
			}
		}
		else if (is16bpc()) {
			uint16_t* q = reinterpret_cast<uint16_t*>(this->getLine(iLine));
			for (size_t iSample = 0; iSample < header.width*header.nChannels; iSample++)
			{
				*p++ = static_cast<float>(*q++) / 65535.0f;
			}
		}
	}
	if (inPlace)
	{
		this->createFrom(out);
		this->iccProfile = out->iccProfile;
		this->pixels.swap(out->pixels);
		delete out;
		out = nullptr;
	}
}

void Image::removeAlphaChannel(bool multiplyOntoColors, Image* out)
{
	if ( !hasAlpha() )
		throw std::runtime_error( "Image does not have an alpha channel" );
	if ( multiplyOntoColors  )
		throw std::runtime_error("multiplyOntoColors not yet supported.");

	bool inPlace = out==nullptr;
	if ( inPlace )
	{
		out = new Image();
	}
	PixelMode newMode = PixelMode::create( pixelMode.isCMYK(), pixelMode.isRGB(), pixelMode.isGray(), pixelMode.isLab(), false, pixelMode.getBitsPerChannel() );
	out->create( this->header.width, this->header.height, newMode, this->dpiX, this->dpiY );
	out->iccProfile = this->iccProfile;
				 
	int bytesPerSample = header.depth/8; 
	for ( size_t iLine=0; iLine < header.height; iLine++ )
	{
		uint8_t* p = reinterpret_cast<uint8_t*>(out->getLine(iLine));
		uint8_t* q = reinterpret_cast<uint8_t*>(this->getLine(iLine));
		for ( size_t iCol = 0; iCol < header.width; iCol++ )
		{
			for ( int iSample = 0; iSample<header.nChannels; iSample++ )
			{
				if ( iSample!=header.alphaChannel ) 
				{
					// non-alpha
					for ( int iByte=0; iByte<bytesPerSample; iByte++ )
						*p++ = *q++;
				}
				else
				{
					// alpha -- just skip those bytes on the input
					q += bytesPerSample;
				}
			}
		}
	}
	if ( inPlace )
	{
		this->createFrom(out);
		this->iccProfile = out->iccProfile;
		this->pixels.swap( out->pixels );
		delete out;
		out = nullptr;
	}	
}

void Image::addAlphaChannel(double transparency/*=0.0*/, Image* out/*=nullptr */)
{
	if (hasAlpha())
		throw std::runtime_error("Image does already have an alpha channel");

	// make sure alpha in valid range
	double opacity = 1.0 - transparency;
	if (opacity < 0.0)
		opacity = 0.0;
	else if (opacity > 1.0)
		opacity = 1.0;

	bool inPlace = out == nullptr;
	if (inPlace)
	{
		out = new Image();
	}
	PixelMode newMode = PixelMode::create(pixelMode.isCMYK(), pixelMode.isRGB(), pixelMode.isGray(), pixelMode.isLab(), true, pixelMode.getBitsPerChannel());
	out->create(this->header.width, this->header.height, newMode, this->dpiX, this->dpiY);
	out->iccProfile = this->iccProfile;

	std::unique_ptr<uint8_t[]> alphaValue;
	if (is8bpc())
	{
		alphaValue.reset(new uint8_t[1]);
		*(alphaValue.get()) = static_cast<uint8_t>(255.0*opacity + 0.5);
	}
	else if (is16bpc())
	{
		alphaValue.reset(new uint8_t[2]);
		*(reinterpret_cast<uint16_t*>(alphaValue.get())) = static_cast<uint16_t>(65535.0*opacity + 0.5);
	}
	else if (is32bpc())
	{
		alphaValue.reset(new uint8_t[4]);
		*(reinterpret_cast<float*>(alphaValue.get())) = static_cast<float>(opacity);
	}

	int bytesPerSample = getChannelSize();
	for (size_t iLine = 0; iLine < header.height; iLine++)
	{
		uint8_t* p = reinterpret_cast<uint8_t*>(out->getLine(iLine));
		uint8_t* q = reinterpret_cast<uint8_t*>(this->getLine(iLine));
		for (size_t iCol = 0; iCol < header.width; iCol++)
		{
			for (int iSample = 0; iSample < out->header.nChannels; iSample++)
			{
				if (iSample != out->header.alphaChannel)
				{
					// non-alpha
					for (int iByte = 0; iByte < bytesPerSample; iByte++)
						*p++ = *q++;
				}
				else
				{
					// alpha -- add the pre-calculated value
					for (int iByte = 0; iByte < bytesPerSample; iByte++)
						*p++ = alphaValue[iByte];
				}
			}
		}
	}

	if (inPlace)
	{
		this->createFrom(out);
		this->iccProfile = out->iccProfile;
		this->pixels.swap(out->pixels);
		delete out;
		out = nullptr;
	}

}


std::vector<Image> Image::separate() const
{
	std::vector<Image> separations( header.nChannels );
	for ( int channel = 0; channel < header.nChannels; channel++ )
	{
		PixelMode pm = PixelMode::create( false, false, true, false, false, header.depth );
		separations[channel].create( getWidth(), getHeight(), pm, getResolutionX(), getResolutionY() );
		
		switch ( header.bytesPerPixel ) {
		case 1:
			for (size_t row = 0; row < getHeight(); row++)
			{
				const uint8_t* src = getLineConst(row) + channel;
				uint8_t* dst = separations[channel].getLine(row);
				for (size_t col = 0; col < getWidth(); col++)
				{
					*dst++ = *src;
					src += header.nChannels;
				}
			}
			break;
		case 2:
			for ( size_t row=0; row<getHeight(); row++ )
			{
				const uint16_t* src = reinterpret_cast<const uint16_t*>(getLineConst(row)) + channel;
				uint16_t* dst = reinterpret_cast<uint16_t*>(separations[channel].getLine(row));
				for ( size_t col=0; col<getWidth(); col++ )
				{
					*dst++ = *src;
					src += header.nChannels;
				}
			}
			break;
		case 4:
			for (size_t row = 0; row < getHeight(); row++)
			{
				const float* src = reinterpret_cast<const float*>(getLineConst(row)) + channel;
				float* dst = reinterpret_cast<float*>(separations[channel].getLine(row));
				for (size_t col = 0; col < getWidth(); col++)
				{
					*dst++ = *src;
					src += header.nChannels;
				}
			}
			break;

		}
	}
	return separations;
}
void Image::combine( PixelMode targetPixelMode, const std::vector<Image>& separations)
{
	int numSep = static_cast<int>( separations.size() );
	
	if ( numSep < 1 )
		throw std::runtime_error( "combine failed: No separations" );
		
	// check if this works ...
	size_t w=separations[0].getWidth(), h=separations[0].getHeight();
	int  d=separations[0].header.depth;
	for ( int i=0; i<numSep; i++ )
	{
		if ( ( separations[1].header.nChannels != 1 ) 
			|| ( (i>0) && (separations[i].getWidth() != w || separations[i].getHeight() != h || separations[i].header.depth != d) ) )
		{ 
			throw std::runtime_error( "combine failed: size or depth of separations differ or pne of the separations has more than 1 channel" );
		}
	}
	
	// create the target image
	create( w, h, targetPixelMode, separations[0].getResolutionX(), separations[0].getResolutionY() );
	if ( header.nChannels != numSep )
	{
		throw std::runtime_error( "combine failed: wrong number of separations for target pixel mode" );
	}
	if ( header.depth != d )
	{
		throw std::runtime_error( "combine failed: separated planes do not match target Pixel Mode's depth" );
	}
	
	// DO IT NOW
	for ( int channel = 0; channel < header.nChannels; channel++ )
	{
		switch ( header.bytesPerPixel ) 
		{
		case 1:
			for (size_t row = 0; row < getHeight(); row++)
			{
				const uint8_t* src = reinterpret_cast<const uint8_t*>(separations[channel].getLineConst(row));
				uint8_t* dst = reinterpret_cast<uint8_t*>(getLine(row)) + channel;
				for (size_t col = 0; col < getWidth(); col++)
				{
					*dst = *src++;
					dst += header.nChannels;
				}
			}
			break;
		case 2:
			for ( size_t row=0; row<getHeight(); row++ )
			{
				const uint16_t* src = reinterpret_cast<const uint16_t*>(separations[channel].getLineConst(row));
				uint16_t* dst = reinterpret_cast<uint16_t*>(getLine(row)) + channel;
				for ( size_t col=0; col<getWidth(); col++ )
				{
					*dst = *src++;
					dst += header.nChannels;
				}
			}
			break;
		case 4:
			for (size_t row = 0; row < getHeight(); row++)
			{
				const float* src = reinterpret_cast<const float*>(separations[channel].getLineConst(row));
				float* dst = reinterpret_cast<float*>(getLine(row)) + channel;
				for (size_t col = 0; col < getWidth(); col++)
				{
					*dst = *src++;
					dst += header.nChannels;
				}
			}
			break;

		}
	}
}
void Image::flip(FlipDirection flipAxis, Image* out)
{
	bool inPlace = out==nullptr;
	if ( inPlace )
	{
		out = new Image();
	}

	if ( flipAxis == FlipDirection::SwapXY )
	{
		out->create( this->header.height, this->header.width, pixelMode, this->dpiY, this->dpiX );
		for ( size_t row=0; row<out->getHeight(); row++ )
		{
			uint8_t* dst = out->getLine(row);
			for ( size_t col=0; col<out->getWidth(); col++ )
			{
				memcpy( dst, this->getPixel(col,row), header.bytesPerPixel );
				dst += header.bytesPerPixel;
			}
		}
	}
	if ( flipAxis == FlipDirection::Horizontal )
	{
		out->create( this->header.width, this->header.height, pixelMode, this->dpiY, this->dpiX );
		for ( size_t row=0; row<out->getHeight(); row++ )
		{
			uint8_t* dst = out->getLine(row);
			for ( size_t col=0; col<out->getWidth(); col++ )
			{
				memcpy( dst, this->getPixel(row, out->getWidth()-col-1), header.bytesPerPixel );
				dst += header.bytesPerPixel;
			}
		}
	}
	if ( flipAxis == FlipDirection::Vertical )
	{
		out->create( this->header.width, this->header.height, pixelMode, this->dpiY, this->dpiX );
		for ( size_t row=0; row<out->getHeight(); row++ )
		{
			uint8_t* dst = out->getLine(row);
			memcpy( dst, this->getLine(out->getHeight()-row-1), header.widthStep );
		}
	}
	if ( inPlace )
	{
		this->createFrom(out);
		this->iccProfile = out->iccProfile;
		this->pixels.swap( out->pixels );
		delete out;
		out = nullptr;
	}	
}

const std::string & Image::getIccProfileContent() const
{
	return iccProfile;
}

void Image::setIccProfile(const std::string iccProfile)
{
	this->iccProfile.assign(iccProfile);
}

void Image::setIccProfile(const void* buffer, size_t len)
{
	iccProfile.assign( reinterpret_cast<const char*>(buffer), len);
}

bool Image::hasIccProfile() const
{
	return !iccProfile.empty();
}

static void fnv_1a( uint64_t & hash, const void* const data, size_t len ) {
	static const uint64_t FNV_prime_64bit = 0x100000001b3;
	const uint8_t * p = reinterpret_cast<const uint8_t*>(data);
	for (size_t i = 0; i < len; i++) {
		hash ^= *p++;
		hash *= FNV_prime_64bit;
	}

}
static void fnv_1a(uint64_t & hash, const std::string & data)
{
	fnv_1a(hash, data.data(), data.length());
}

uint64_t Image::calcHash(bool includeIccProfile /*= true*/) const
{
	static const uint64_t FNV_offset_basis_64bit = 0xcbf29ce484222325;
	uint64_t hash = FNV_offset_basis_64bit;
	
	// header ...
//	std::cerr << "Header size: " << header.nSize << std::endl;
//	std::cerr << "string size: " << sizeof(std::string) << std::endl;
//	fnv_1a(hash, &header.nSize, sizeof(header.nSize)); size may differe ...
	fnv_1a(hash, &header.ID, sizeof(header.ID));
	fnv_1a(hash, &header.nChannels, sizeof(header.nChannels));
	fnv_1a(hash, &header.alphaChannel, sizeof(header.alphaChannel));
	fnv_1a(hash, &header.depth, sizeof(header.depth));
	fnv_1a(hash, header.colorModel);
	fnv_1a(hash, header.channelSeq);
	fnv_1a(hash, &header.dataOrder, sizeof(header.dataOrder));
	fnv_1a(hash, &header.origin, sizeof(header.origin));
//	fnv_1a(hash, &header.pixelAlign, sizeof(header.pixelAlign));
//	fnv_1a(hash, &header.lineAlign, sizeof(header.lineAlign));
	fnv_1a(hash, &header.bytesPerPixel, sizeof(header.bytesPerPixel));
	fnv_1a(hash, &header.width, sizeof(header.width));
	fnv_1a(hash, &header.height, sizeof(header.height));
//	fnv_1a(hash, &header.widthStep, sizeof(header.widthStep));
//	fnv_1a(hash, &header.imageSize, sizeof(header.imageSize));
	int pm = pixelMode;
	fnv_1a(hash, &pm, sizeof(pm));
	fnv_1a(hash, &dpiX, sizeof(dpiX));
	fnv_1a(hash, &dpiY, sizeof(dpiY));

	size_t significantLineLength = header.bytesPerPixel*header.width;
//	std::cerr << "significantLineLength size: " << significantLineLength << std::endl;
//	std::cerr << "header widthStep: " << header.widthStep << std::endl;
//	std::cerr << "header color model: " << header.colorModel << std::endl;
//	std::cerr << "header width: " << header.width << std::endl;
//	std::cerr << "header height: " << header.height << std::endl;
	uint8_t* p = pixels.get();
	for (size_t line = 0; line < header.height; line++) {
		fnv_1a(hash, p, significantLineLength );
		p += header.widthStep;
	}

	if (includeIccProfile) {
		fnv_1a(hash, getIccProfileContent());
	}

	return hash;
}

