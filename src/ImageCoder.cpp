/* ImageCoder.cpp
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
#include "ImageCoder.h"
#include "ImageCoderBitmap.h"
#include "ImageCoderJpeg.h"
#include "ImageCoderJpeg2000.h"
#include "ImageCoderTiff.h"
#include "ImageCoderPcx.h"
#include "ImageCoderPng.h"
#include "ImageCoderGif.h"
#include "ImageCoderTarga.h"


ImageCoder::ImageCoder( ImageEncoding enc, Image* img ) : encoderType(enc), image(img), props(nullptr)
{
}


ImageCoder::~ImageCoder()
{
}

std::string & ImageCoder::getIccProfile()
{
	return image->iccProfile;
}

ImageEncoding ImageCoder::determineImageType(std::istream & stream)
{
	uint8_t byte[17];
	stream.read(reinterpret_cast<char*>(byte), 4);
	// JPEG ?
	if (byte[0] == 0xff && byte[1] == 0xd8 && byte[2] == 0xff )
	{
		stream.seekg(0);
		return IE_JPEG;
	}
/*	if (byte[0] == 0xff && byte[1] == 0xd8 && byte[2] == 0xff && byte[3] == 0xe0)
	{
		stream.read(reinterpret_cast<char*>(byte + 4), 6);
		if (byte[6] == 'J' && byte[7] == 'F' && byte[8] == 'I' && byte[9] == 'F')
		{
			stream.seekg(0);
			return IE_JPEG;
		}
	}
*/
	// GIF ?
	else if (byte[0] == 'G' && byte[1] == 'I' && byte[2] == 'F' && byte[3] == '8')
	{
		stream.read(reinterpret_cast<char*>(byte + 4), 2);
		if ((byte[4] == '7' || byte[4] == '9') && byte[5] == 'a')
		{
			stream.seekg(0);
			return IE_GIF;
		}
	}
	// PNG ? 
	else if (byte[0] == 0x89 && byte[1] == 'P' && byte[2] == 'N' && byte[3] == 'G')
	{
		stream.read(reinterpret_cast<char*>(byte + 4), 4);
		if ( byte[4] == '\r' && byte[5] == '\n' && byte[6] == 0x1a && byte[7] == '\n' )
		{
			stream.seekg(0);
			return IE_PNG;
		}
	}
	// TIFF Intel ?
	else if (byte[0] == 'I' && byte[1] == 'I' && byte[2] == 42 && byte[3] == 0 )
	{
		stream.seekg(0);
		return IE_TIFF;
	}
	// TIFF Motorola ?
	else if (byte[0] == 'M' && byte[1] == 'M' && byte[2] == 0 && byte[3] == 42 )
	{
		stream.seekg(0);
		return IE_TIFF;
	}
	//BMP ?
	else if (byte[0] == 'B' && byte[1] == 'M' )
	{
		stream.seekg(0);
		return IE_BMP;
	}
	// JPEG2000 ?
	else if (byte[0] == 0x00 && byte[1] == 0x00 && byte[2] == 0x00 && byte[3] == 0x0c)
	{
		stream.read(reinterpret_cast<char*>(byte + 4), 6);
		if (byte[4] == 'j' && byte[5] == 'P' && byte[6] == ' ' && byte[7] == ' ' && byte[8] == '\r' && byte[9] == '\n')
		{
			stream.seekg(0);
			return IE_JPEG2000;
		}
	}
	else if (byte[0] == 0x0a && byte[1] <= 5 && byte[1] != 1 && byte[2] <= 1 && (byte[3] == 1 || byte[3] == 2 || byte[3] == 8))
	{
		stream.seekg(0);
		return IE_PCX;
	}
	
	stream.seekg(0);
	throw std::runtime_error("unknown image encoding in file.");
}


ImageCoder* ImageCoder::createCoder(ImageEncoding encoding, Image* img, ImageCoderProperties* props)
{
	ImageCoder* coder = nullptr;
	switch (encoding)
	{
	case IE_BMP:
		coder = new ImageCoderBitmap(img);
		break;
	case IE_PNG:
		coder = new ImageCoderPng(img);
		break;
	case IE_GIF:
		coder = new ImageCoderGif(img);
		break;
	case IE_JPEG:
		coder = new ImageCoderJpeg(img);
		break;
	case IE_JPEG2000:
		coder = new ImageCoderJpeg2000(img);
		break;	
	case IE_TIFF:
		coder = new ImageCoderTiff(img);
		break;
	case IE_PCX:
		coder = new ImageCoderPcx(img);
		break;
	case IE_TGA:
		coder = new ImageCoderTarga(img);
		break;
	}
	if ( coder )
		coder->props = props;
	return coder;
	
}

bool ImageCoder::onSubImageRead(size_t subImage)
{
	return image->imageReadCallback( subImage );
}
/*
void ImageCoder::addParameter( ImageCoderParameter* parameter )
{
	// make sure that it didn't exist before ? ?????
	parameterMap[parameter->getParameterType()] = parameter;
}
ImageCoderParameter * ImageCoder::getParameter(ImageCoderParameterType parameterType)
{
	auto it = parameterMap.find( parameterType );
	if ( it == parameterMap.end() )
		return nullptr;
	return it->second;
}
*/