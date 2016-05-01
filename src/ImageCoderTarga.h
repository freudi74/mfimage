/* ImageCoderTarga.h
 * Copyright 2016 Marco Freudenberger.
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
	virtual void read(std::istream & stream) override;
	virtual void write(const std::string & filename) override;
	virtual void write(std::ostream & stream) override;

public:
	static bool mightBeTargaHeader(uint8_t* byte18);
private:
	uint32_t readNextPixel(std::istream & stream, size_t bytesPerPixel, bool rle);
	int scale5To8bit(int _5bitVal);
	// rle buffer
	uint32_t rleLastVal;
	int rleRepeatsLeft;
	int rleRawLeft;
};

