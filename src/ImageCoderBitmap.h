/* ImageCoderBitmap.h
 * Copyright 2015 Marco Freudenberger.
 * 
 * This file is part of mfimage library.
 * 
 * Design and implementation by:
 * --------------------------------------------------
 * - Marco Freudenberger (Marco.Freudenberger@gmx.de)
 *
 */

#pragma once
#include "ImageCoder.h"
#include <list>

class ImageCoderBitmap :
	public ImageCoder
{
public:
	ImageCoderBitmap( Image* img );
	virtual ~ImageCoderBitmap();

	virtual bool canDecode() override;
	virtual bool canEncode() override;
	virtual bool canEncode( PixelMode pixelMode ) override;
	virtual void read( const std::string & filename ) override;
	virtual void read( std::istream & stream ) override;
	virtual void write( const std::string & filename) override;
	virtual void write( std::ostream & stream ) override;


private:
	unsigned char getColorIndex( const unsigned char* line, size_t pixelNo, uint32_t bitsPerPixel );
};
