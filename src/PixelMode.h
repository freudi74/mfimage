/* PixelMode.h
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
#include <cstdint>
#include <string>

/**
 * @class PixelMode
 * @author Marco Freudenberger
 * @date 09/23/15
 * @file PixelMode.h
 * @brief 
 */
class PixelMode
{
public:
	typedef enum PixelModeEnum {
		GRAY8  = 0x01,	
		GRAY16,
		GRAY32,
		AGRAY8 = 0x09,
		AGRAY16,
		AGRAY32,
		RGB8   = 0x11,
		RGB16,
		RGB32,
		ARGB8  = 0x19,
		ARGB16,
		ARGB32,
		CMYK8  = 0x21,
		CMYK16,
		CMYK32,
		CMYKA8 = 0x29,
		CMYKA16,
		CMYKA32,
		LAB32  = 0x33,
		LABA32 = 0x3B
	} PixelModeEnum;
	
	// note: in all float ranges, any value is between 0.0 and 1.0.
	// exception: LAB: L=0.0-100.0; a,b = -something to +something; most often, between -128.0 and +127.0, but this is an "artificial" limitation based on 8 and 16 bit file encodings.
	
	PixelMode( PixelModeEnum val=RGB8 );
	virtual ~PixelMode();
	static PixelMode create(const std::string & colorModel, bool hasAlpha, uint32_t _bitsPerChannel);
	static PixelMode create(bool isCMYK, bool isRGB, bool isGray, bool isLab, bool hasAlpha, uint32_t _bitsPerChannel);
	static PixelModeEnum compose(const std::string & colorModel, bool hasAlpha, uint32_t _bitsPerChannel);
	static PixelModeEnum compose(bool isCMYK, bool isRGB, bool isGray, bool isLab, bool hasAlpha, uint32_t _bitsPerChannel);

	inline bool isCMYK() const { return _isCMYK; }
	inline bool isRGB() const { return _isRGB; }
	inline bool isGray() const { return _isGray; }
	inline bool isLab() const { return _isLab; }
	inline bool hasAlpha() const { return _hasAlpha; }
	inline uint32_t getBitsPerChannel() const { return _bitsPerChannel; }
	inline uint32_t getNumChannels() const { return _numChannels;  }
	
	uint32_t getLittleCmsFormat() const;
	operator int() const { return value; } // allows simple switch( pixelMode )
	
private:	
	PixelModeEnum value = RGB8;
	bool _isCMYK = false;
	bool _isRGB = true;
	bool _isGray = false;
	bool _isLab = false;
	bool _hasAlpha = false;
	uint32_t _numChannels = 3; // including alpha.
	uint32_t _bitsPerChannel = 8;
};
