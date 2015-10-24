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

void ImageCoderTarga::read(const std::string& filename)
{
	// todo!
}

void ImageCoderTarga::write(const std::string& filename)
{
	throw std::runtime_error("JPEG write not implemented");		
}
