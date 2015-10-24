/* ImageCoderTarga.h
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

class ImageCoderTarga : 
	public ImageCoder
{
public:
	ImageCoderTarga(Image* img);
	virtual ~ImageCoderTarga();

public:
	virtual bool canDecode() override;
	virtual bool canEncode() override;
	virtual bool canEncode( PixelMode pixelMode ) override;
	virtual void read(const std::string& filename) override;
	virtual void write(const std::string& filename) override;
};

