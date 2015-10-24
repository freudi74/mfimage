/* ImageCoderPcx.h
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

class ImageCoderPcx :
	public ImageCoder
{
public:
	ImageCoderPcx(Image* img);
	virtual ~ImageCoderPcx();

	virtual bool canDecode() override;
	virtual bool canEncode() override;
	virtual bool canEncode( PixelMode pixelMode ) override;
	virtual void read(const std::string & filename) override;
	virtual void write(const std::string & filename) override;

private:
	void readNextPixel(std::ifstream & f, bool isRLE, uint8_t & pixelValue, uint8_t & runCount);
};

