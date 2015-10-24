/* ImageCoder.h
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
#include <list>
#include <map>
#include <iostream>
#include "Image.h"
#include "ImageCoderProperties.h"

class ImageCoder
{
protected:
	ImageCoder(ImageEncoding enc, Image* img );
public:
	virtual ~ImageCoder();

	virtual bool canDecode() = 0;
	virtual bool canEncode() = 0;
	virtual bool canEncode( PixelMode pixelMode ) = 0;
	virtual void read(const std::string & filename ) = 0;
//  virtual void read(std::istream & stream) = 0;
	virtual void write(const std::string & filename ) = 0;
//  virtual void write(std::ostream & stream) = 0;
	
	static ImageCoder* createCoder( ImageEncoding encoding, Image* img, ImageCoderProperties* props );
	static ImageEncoding determineImageType( std::istream & stream );
	
	

protected:
	ImageEncoding         encoderType;
	Image*                image;
	ImageCoderProperties* props;
	
	// tools due to bad design ...
	std::string & getIccProfile();
	bool onSubImageRead(size_t subImage);
};

