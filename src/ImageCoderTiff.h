/* ImageCoderTiff.h
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
#include "ImageCoderParameter.h"

typedef struct tiff TIFF; // fwd decl
class ImageCoderTiff :
	public ImageCoder
{
public:
	ImageCoderTiff(Image* img);
	virtual ~ImageCoderTiff();
	
	virtual bool canDecode() override;
	virtual bool canEncode() override;
	virtual bool canEncode( PixelMode pixelMode ) override;
	virtual void read(const std::string & filename) override;
	virtual void write(const std::string & filename) override;

protected:
	void readBilevelOrGray(TIFF* tif, uint32_t width, uint32_t height, double dpiX, double dpiY, uint16_t samplesPerPixel, uint16_t bitsPerSample, int extraAlphaSample, bool inverted, bool separated, uint16_t fillorder, uint16_t whiteLevel, uint16_t blackLevel, uint8_t* tiledImageBuffer );
	void readRGB(TIFF* tif, uint32_t width, uint32_t height, double dpiX, double dpiY, uint16_t samplesPerPixel, uint16_t bitsPerSample, int extraAlphaSample, bool separated, uint8_t* tiledImageBuffer);
	void readCMYK(TIFF* tif, uint32_t width, uint32_t height, double dpiX, double dpiY, uint16_t samplesPerPixel, uint16_t bitsPerSample, int extraAlphaSample, bool separated, uint8_t* tiledImageBuffer);
	void readRGBPalette(TIFF* tif, uint32_t width, uint32_t height, double dpiX, double dpiY, uint16_t samplesPerPixel, uint16_t bitsPerSample, int extraAlphaSample, bool separated, uint8_t* tiledImageBuffer);
};
