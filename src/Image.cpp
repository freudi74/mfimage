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




class colorModel;
Image::Image() : pixelMode(PixelMode::ARGB8)
{
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
		header.channelSeq = "G";
		header.bytesPerPixel = 1;
		break;
	case PixelMode::GRAY16:
		header.nChannels = 1;
		header.depth = 16;
		header.alphaChannel = -1;
		header.colorModel = ColorModel_Gray;
		header.channelSeq = "G";
		header.bytesPerPixel = 2;
		break;
	case PixelMode::AGRAY8:
		header.nChannels = 2;
		header.depth = 8;
		header.alphaChannel = 0;
		header.colorModel = ColorModel_Gray;
		header.channelSeq = "AG";
		header.bytesPerPixel = 2;
		break;
	case PixelMode::AGRAY16:
		header.nChannels = 2;
		header.depth = 16;
		header.alphaChannel = 0;
		header.colorModel = ColorModel_Gray;
		header.channelSeq = "AG";
		header.bytesPerPixel = 4;
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
	if ( other->header.width != header.width || other->header.height != header.height || other->header.depth != header.depth )
		throw std::runtime_error( "copyAlphaFrom failed, src image has different size or sampleDepth" );
	if ( ! other->hasAlpha()  )
		throw std::runtime_error( "copyAlphaFrom failed, src image has no Alpha Channel" );
	if ( ! this->hasAlpha()  )
		throw std::runtime_error( "copyAlphaFrom failed, destination image has no Alpha Channel" );
	size_t bytesPerSample = header.depth / 8;
	for ( size_t iLine=0; iLine < header.height; iLine++ )
	{
		for ( size_t iRow=0; iRow < header.width; iRow++ )
		{
			uint8_t* dst = reinterpret_cast<uint8_t*>( this->getPixel( iLine, iRow ) );
			uint8_t* src = reinterpret_cast<uint8_t*>( const_cast<Image*>(other)->getLine(iLine) );
			for ( size_t iByte=0; iByte<bytesPerSample; iByte++ )
				dst[header.alphaChannel*bytesPerSample+iByte] = src[header.alphaChannel*bytesPerSample+iByte];
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

uint8_t* Image::getPixel(size_t line, size_t column)
{
	return getPixel( getLine(line), column );
}

uint8_t* Image::getPixel(void* line, size_t column)
{
	// UNCHECKED ! column < header.width !
	return reinterpret_cast<uint8_t*>(line) + (header.bytesPerPixel * column);
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

bool Image::hasAlpha() const
{
	return pixelMode.hasAlpha();
}

bool Image::is16bpc() const
{
	return pixelMode.getBpc()==16;
}

bool Image::is8bpc() const
{
	return pixelMode.getBpc()==8;
}

/* Fill image with given "Color" */
void Image::fill(void* pixel, size_t x, size_t y, size_t w, size_t h )
{
	if ( w==0 ) w = header.width - x;
	if ( h==0 ) h = header.height - y;
	
	uint8_t* q = reinterpret_cast<uint8_t*>(pixel);
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

	bool inPlace = out==nullptr;
	if ( inPlace )
	{
		out = new Image();
	}
	PixelMode newMode = PixelMode::create( pixelMode.isCMYK(), pixelMode.isRGB(), pixelMode.isGray(), pixelMode.hasAlpha(), 16 );
	out->create( this->header.width, this->header.height, newMode, this->dpiX, this->dpiY );
	out->iccProfile = this->iccProfile;
				 
	for ( size_t iLine=0; iLine < header.height; iLine++ )
	{
		uint16_t* p = reinterpret_cast<uint16_t*>(out->getLine(iLine));
		uint8_t* q = reinterpret_cast<uint8_t*>(this->getLine(iLine));
		for ( size_t iSample = 0; iSample < header.width*header.nChannels; iSample++ )
		{
			*p++ = static_cast<uint16_t>( (*q++) << 8 );
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

	bool inPlace = out==nullptr;
	if ( inPlace )
	{
		out = new Image();
	}
	PixelMode newMode = PixelMode::create( pixelMode.isCMYK(), pixelMode.isRGB(), pixelMode.isGray(), pixelMode.hasAlpha(), 8 );
	out->create( this->header.width, this->header.height, newMode, this->dpiX, this->dpiY );
	out->iccProfile = this->iccProfile;
				 
	for ( size_t iLine=0; iLine < header.height; iLine++ )
	{
		uint8_t* p = reinterpret_cast<uint8_t*>(out->getLine(iLine));
		uint16_t* q = reinterpret_cast<uint16_t*>(this->getLine(iLine));
		for ( size_t iSample = 0; iSample < header.width*header.nChannels; iSample++ )
		{
			*p++ = static_cast<uint8_t>( (*q++) >> 8 );
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
void Image::removeAlphaChannel(bool multipleOntoColors, Image* out)
{
	if ( !hasAlpha() )
		throw std::runtime_error( "Image does not have an alpha channel" );
		
	bool inPlace = out==nullptr;
	if ( inPlace )
	{
		out = new Image();
	}
	PixelMode newMode = PixelMode::create( pixelMode.isCMYK(), pixelMode.isRGB(), pixelMode.isGray(), false, pixelMode.getBpc() );
	out->create( this->header.width, this->header.height, newMode, this->dpiX, this->dpiY );
	out->iccProfile = this->iccProfile;
				 
	int bytesPerSample = header.depth/8; // works only for 8 and 16!
	for ( size_t iLine=0; iLine < header.height; iLine++ )
	{
		uint8_t* p = reinterpret_cast<uint8_t*>(out->getLine(iLine));
		uint8_t* q = reinterpret_cast<uint8_t*>(this->getLine(iLine));
		for ( size_t iCol = 0; iCol < header.width; iCol++ )
		{
			for ( int iSample = 0; iSample<header.nChannels; iSample++ )
			{
				if ( iSample > 0 ) 
				{
					// non-alpha
					for ( int iByte=0; iByte<bytesPerSample; iByte++ )
						*p++ = *q++;
				}
				else
				{
					// alpha
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
std::vector<Image> Image::separate() const
{
	std::vector<Image> separations( header.nChannels );
	for ( int channel = 0; channel < header.nChannels; channel++ )
	{
		PixelMode pm = PixelMode::create( false, false, true, false, header.depth );
		separations[channel].create( getWidth(), getHeight(), pm, getResolutionX(), getResolutionY() );
		
		if ( header.bytesPerPixel == 2 )
		{
			for ( size_t row=0; row<getHeight(); row++ )
			{
				const uint16_t* src = reinterpret_cast<const uint16_t*>(const_cast<Image*>(this)->getLine(row)) + channel;
				uint16_t* dst = reinterpret_cast<uint16_t*>(separations[channel].getLine(row));
				for ( size_t col=0; col<getWidth(); col++ )
				{
					*dst++ = *src;
					src += header.nChannels;
				}
			}
		} 
		else
		{
			for ( size_t row=0; row<getHeight(); row++ )
			{
				const uint8_t* src = const_cast<Image*>(this)->getLine(row) + channel;
				uint8_t* dst = separations[channel].getLine(row);
				for ( size_t col=0; col<getWidth(); col++ )
				{
					*dst++ = *src;
					src += header.nChannels;
				}
			}
		}
	}
	return separations;
}
void Image::combine( PixelMode targetPixelMode, const std::vector<Image>& separations)
{
	int numSep = separations.size();
	
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
		if ( header.bytesPerPixel == 2 )
		{
			for ( size_t row=0; row<getHeight(); row++ )
			{
				const uint16_t* src = reinterpret_cast<const uint16_t*>(const_cast<Image*>(&separations[channel])->getLine(row));
				uint16_t* dst = reinterpret_cast<uint16_t*>(getLine(row)) + channel;
				for ( size_t col=0; col<getWidth(); col++ )
				{
					*dst = *src++;
					dst += header.nChannels;
				}
			}
		} 
		else
		{
			for ( size_t row=0; row<getHeight(); row++ )
			{
				const uint8_t* src = reinterpret_cast<const uint8_t*>(const_cast<Image*>(&separations[channel])->getLine(row));
				uint8_t* dst = reinterpret_cast<uint8_t*>(getLine(row)) + channel;
				for ( size_t col=0; col<getWidth(); col++ )
				{
					*dst = *src++;
					dst += header.nChannels;
				}
			}
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
