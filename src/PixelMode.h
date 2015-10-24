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
		GRAY8 = 1,	
		GRAY16,
		AGRAY8,
		AGRAY16,
		RGB8,
		RGB16,
		ARGB8,
		ARGB16,
		CMYK8,
		CMYK16,
		CMYKA8,
		CMYKA16	
	} PixelModeEnum;
	
	PixelMode( PixelModeEnum val=RGB8 );
	virtual ~PixelMode();
	static PixelMode create( bool isCMYK, bool isRGB, bool isGray, bool hasAlpha, uint32_t bpc );

	inline bool isCMYK() const { return _isCMYK; }
	inline bool isRGB() const { return _isRGB; }
	inline bool isGray() const { return _isGray; }
	inline bool hasAlpha() const { return _hasAlpha; }
	inline uint32_t getBpc() const { return _bpc; }
	
	uint32_t getLittleCmsFormat() const;
	operator int() const { return value; } // allows simple switch( pixelMode )
	
private:	
	PixelModeEnum value = RGB8;
	bool _isCMYK = false;
	bool _isRGB = true;
	bool _isGray = false;
	bool _hasAlpha = false;
	uint32_t _bpc = 8;
};
