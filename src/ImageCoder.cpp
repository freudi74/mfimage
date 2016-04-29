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
#include "userCoders/UserCoder_Springboard.h"

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
	uint8_t byte[18];
	std::streampos originalPos = stream.tellg();
	stream.read(reinterpret_cast<char*>(byte), 4);
	
	/////////////////////////////////
	// Jpeg ?
	if (byte[0] == 0xff && byte[1] == 0xd8 && byte[2] == 0xff )
	{
		stream.seekg(originalPos);
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
	////////////////////////////
	// GIF ?
	else if (byte[0] == 'G' && byte[1] == 'I' && byte[2] == 'F' && byte[3] == '8')
	{
		stream.read(reinterpret_cast<char*>(byte + 4), 2);
		if ((byte[4] == '7' || byte[4] == '9') && byte[5] == 'a')
		{
			stream.seekg(originalPos);
			return IE_GIF;
		}
		stream.seekg(-4, stream.cur);
	}

	////////////////////////////
	// PNG ? 
	else if (byte[0] == 0x89 && byte[1] == 'P' && byte[2] == 'N' && byte[3] == 'G')
	{
		stream.read(reinterpret_cast<char*>(byte + 4), 4);
		if ( byte[4] == '\r' && byte[5] == '\n' && byte[6] == 0x1a && byte[7] == '\n' )
		{
			stream.seekg(originalPos);
			return IE_PNG;
		}
		stream.seekg(-4, stream.cur);
	}

	////////////////////////////
	// TIFF ?
	else if (byte[0] == 'I' && byte[1] == 'I' && byte[2] == 42 && byte[3] == 0 ) // TIFF Intel ?
	{
		stream.seekg(originalPos);
		return IE_TIFF;
	}
	else if (byte[0] == 'M' && byte[1] == 'M' && byte[2] == 0 && byte[3] == 42 ) // TIFF Motorola ?
	{
		stream.seekg(originalPos);
		return IE_TIFF;
	}

	////////////////////////////
	// BMP ?
	else if (byte[0] == 'B' && byte[1] == 'M' )
	{
		stream.seekg(originalPos);
		return IE_BMP;
	}

	///////////////////////////
	// JPEG2000 ?
	else if (byte[0] == 0x00 && byte[1] == 0x00 && byte[2] == 0x00 && byte[3] == 0x0c)
	{
		stream.read(reinterpret_cast<char*>(byte + 4), 6);
		if (byte[4] == 'j' && byte[5] == 'P' && byte[6] == ' ' && byte[7] == ' ' && byte[8] == '\r' && byte[9] == '\n')
		{
			stream.seekg(originalPos);
			return IE_JPEG2000;
		}
		stream.seekg(-4, stream.cur );

	}

	//////////////////////////
	// PCX ?
	else if (byte[0] == 0x0a && byte[1] <= 5 && byte[1] != 1 && byte[2] <= 1 && (byte[3] == 1 || byte[3] == 2 || byte[3] == 8))
	{
		stream.seekg(originalPos);
		return IE_PCX;
	}

	//////////////////////////
	// user codec ?
	// 
	else {
		std::istream::pos_type pos = stream.tellg();
#ifdef IS_USER_CODEC_1
		if ( IS_USER_CODEC_1(byte, stream) ) {
			stream.seekg(originalPos);
			return IE_USER_1;
		}
		stream.seekg(pos);
#endif
#ifdef IS_USER_CODEC_2
		if ( IS_USER_CODEC_2(byte, stream) ) {
			stream.seekg(originalPos);
			return IE_USER_2;
		}
		stream.seekg(pos);
#endif
#ifdef IS_USER_CODEC_3
		if (IS_USER_CODEC_3(byte, stream)) {
			stream.seekg(originalPos);
			return IE_USER_3;
		}
		stream.seekg(pos);
#endif
#ifdef IS_USER_CODEC_4
		if (IS_USER_CODEC_4(byte, stream)) {
			stream.seekg(originalPos);
			return IE_USER_4;
		}
		stream.seekg(pos);
#endif
#ifdef IS_USER_CODEC_5
		if (IS_USER_CODEC_5(byte, stream)) {
			stream.seekg(originalPos);
			return IE_USER_5;
		}
		stream.seekg(pos);
#endif
#ifdef IS_USER_CODEC_6
		if (IS_USER_CODEC_6(byte, stream)) {
			stream.seekg(originalPos);
			return IE_USER_6;
		}
		stream.seekg(pos);
#endif
#ifdef IS_USER_CODEC_7
		if (IS_USER_CODEC_7(byte, stream)) {
			stream.seekg(originalPos);
			return IE_USER_7;
		}
		stream.seekg(pos);
#endif
#ifdef IS_USER_CODEC_8
		if (IS_USER_CODEC_8(byte, stream)) {
			stream.seekg(originalPos);
			return IE_USER_8;
		}
		stream.seekg(pos);
#endif

		/////////////////////////
		// TGA ? - do this as last one, as this might (TGA V1) need an actual test read of the image,
		// at least it's not clearly identified, so we do a ton of checks ...
		// 
		stream.read(reinterpret_cast<char*>(byte + 4), 14); // read complete targa header
		if ( ImageCoderTarga::mightBeTargaHeader(byte) ) {
			stream.seekg(originalPos);
			return IE_TGA;
		}

	}


	
	stream.seekg(originalPos);
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
#ifdef CREATE_USER_CODER_1
	case IE_USER_1:
		coder = CREATE_USER_CODER_1(img);
		break;
#endif
#ifdef CREATE_USER_CODER_2
	case IE_USER_2:
		coder = CREATE_USER_CODER_2(img);
		break;
#endif
#ifdef CREATE_USER_CODER_3
	case IE_USER_3:
		coder = CREATE_USER_CODER_3(img);
		break;
#endif
#ifdef CREATE_USER_CODER_4
	case IE_USER_4:
		coder = CREATE_USER_CODER_4(img);
		break;
#endif
#ifdef CREATE_USER_CODER_5
	case IE_USER_5:
		coder = CREATE_USER_CODER_5(img);
		break;
#endif
#ifdef CREATE_USER_CODER_6
	case IE_USER_6:
		coder = CREATE_USER_CODER_6(img);
		break;
#endif
#ifdef CREATE_USER_CODER_7
	case IE_USER_7:
		coder = CREATE_USER_CODER_7(img);
		break;
#endif
#ifdef CREATE_USER_CODER_8
	case IE_USER_8:
		coder = CREATE_USER_CODER_8(img);
		break;
#endif
	}

	if ( coder )
		coder->props = props;
	return coder;
	
}

bool ImageCoder::onSubImageRead(size_t subImage)
{
	return image->imageReadCallback( subImage );
}

#define TEMP_FILE_NAME_LENGTH_MAX (L_tmpnam*2)
std::string ImageCoder::storeStreamToTempFile(std::istream & stream)
{
	std::string fileName;
	{
		char fileNameBuffer[L_tmpnam];
		char * result = tmpnam(fileNameBuffer);
		if (result == nullptr)
			return fileName;	// empty file name indicates a problem ...
		fileName.assign(result);
	}

	std::ofstream file(fileName, std::ofstream::binary);
	if (file.fail())
		std::runtime_error("failed to create temp file: '" + fileName + "'");

	try
	{
		file << stream.rdbuf();
		if (file.fail() || stream.fail())
			std::runtime_error("failed to write to temp file: '" + fileName + "'");
	}
	catch (const std::iostream::failure &)
	{
		std::runtime_error("failed to write to temp file: '" + fileName + "', caught failure");
	}
	return fileName;
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