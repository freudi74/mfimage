/* ImageCoderPng.cpp
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

class ImageCoderPng :
	public ImageCoder
{
public:
	ImageCoderPng(Image* img);
	virtual ~ImageCoderPng();

	virtual bool canDecode() override;
	virtual bool canEncode() override;
	virtual bool canEncode( PixelMode pixelMode ) override;
	virtual void read(const std::string & filename) override;
	virtual void write(const std::string & filename) override;

public:
	char* _data = nullptr;
	size_t _dataLeft = 0;
	char* _dataCur = nullptr;
};

