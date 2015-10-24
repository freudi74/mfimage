/* ImageCoderGif.h
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
#include <mutex>

// WARNING! GIFLIB < 5.0 (we currently use v 4.1.6) is NOT REEANTRANT! That's why we have the mutex in the reader...

class ImageCoderGif :
	public ImageCoder
{
public:
	ImageCoderGif(Image* img);
	virtual ~ImageCoderGif();

	virtual bool canDecode() override;
	virtual bool canEncode() override;
	virtual bool canEncode( PixelMode pixelMode ) override;
	virtual void read(const std::string & filename) override;
	virtual void write(const std::string & filename) override;

private:
	static std::mutex s_giflibMutex;
	//std::string getLastErrorText();
	
	typedef struct GraphicsControlExtension {
		bool transparencyFlag = false;
		bool userInputFlag = false;
		uint8_t  disposalMethod = 0; // 0-7
		uint8_t  reserved = 0;       // 0-7
		uint16_t delay10ms = 0; // times 10 ms!
		uint8_t  transparentColorIndex = 0;
	} GraphicsControlExtension;

	void handleGraphicsExtensionBlock( void* extension, int blockNo, GraphicsControlExtension* gce );
};

