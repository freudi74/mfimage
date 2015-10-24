/* PixelMode.cpp
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
#include "PixelMode.h"

#include <lcms2.h>
#include <stdexcept>

PixelMode::PixelMode( PixelModeEnum val )
{
	value = val;
	switch ( value ) {
	case GRAY8:   _isCMYK = false; _isRGB = false; _isGray = true;  _hasAlpha = false; _bpc = 8;  break;
	case GRAY16:  _isCMYK = false; _isRGB = false; _isGray = true;  _hasAlpha = false; _bpc = 16; break;
	case AGRAY8:  _isCMYK = false; _isRGB = false; _isGray = true;  _hasAlpha = true;  _bpc = 8;  break;
	case AGRAY16: _isCMYK = false; _isRGB = false; _isGray = true;  _hasAlpha = true;  _bpc = 16; break;
	case RGB8:    _isCMYK = false; _isRGB = true;  _isGray = false; _hasAlpha = false; _bpc = 8;  break;
	case RGB16:   _isCMYK = false; _isRGB = true;  _isGray = false; _hasAlpha = false; _bpc = 16; break;
	case ARGB8:   _isCMYK = false; _isRGB = true;  _isGray = false; _hasAlpha = true;  _bpc = 8;  break;
	case ARGB16:  _isCMYK = false; _isRGB = true;  _isGray = false; _hasAlpha = true;  _bpc = 16; break;
	case CMYK8:   _isCMYK = true;  _isRGB = false; _isGray = false; _hasAlpha = false; _bpc = 8;  break;
	case CMYK16:  _isCMYK = true;  _isRGB = false; _isGray = false; _hasAlpha = false; _bpc = 16; break;
	case CMYKA8:  _isCMYK = true;  _isRGB = false; _isGray = false; _hasAlpha = true;  _bpc = 8;  break;
	case CMYKA16: _isCMYK = true;  _isRGB = false; _isGray = false; _hasAlpha = true;  _bpc = 16; break;
	}
}

PixelMode::~PixelMode()
{
}
uint32_t PixelMode::getLittleCmsFormat() const
{
	switch ( value ) {
	case GRAY8:   return TYPE_GRAY_8;
	case GRAY16:  return TYPE_GRAY_16;
	case AGRAY8:  return TYPE_GRAYA_8|SWAPFIRST_SH(1);
	case AGRAY16: return TYPE_GRAYA_16|SWAPFIRST_SH(1);
	case RGB8:    return TYPE_RGB_8;
	case RGB16:   return TYPE_RGB_16;
	case ARGB8:   return TYPE_ARGB_8;
	case ARGB16:  return TYPE_ARGB_16;
	case CMYK8:   return TYPE_CMYK_8;
	case CMYK16:  return TYPE_CMYK_16;
	case CMYKA8:  return TYPE_CMYKA_8;
	case CMYKA16: return TYPE_CMYK_16|EXTRA_SH(1);
	// no default to get compiler warning
	}
	return TYPE_RGB_8;
}

PixelMode PixelMode::create(bool isCMYK, bool isRGB, bool isGray, bool hasAlpha, uint32_t bpc)
{
	if ( (isCMYK && isRGB) || (isCMYK && isGray) || (isRGB && isGray) )
		throw std::runtime_error( "composePixelMode failed: more than one color model set" );
	if ( bpc != 8 && bpc != 16 )
		throw std::runtime_error( "composePixelMode failed: unsupported bpc" );
	
	if ( isGray ) {
		switch ( bpc ) {
		case 8:	 return hasAlpha ? AGRAY8 : GRAY8;
		case 16: return hasAlpha ? AGRAY16 : GRAY16;
		}
	}
	if ( isRGB ) {
		switch ( bpc ) {
		case 8:	 return hasAlpha ? ARGB8 : RGB8;
		case 16: return hasAlpha ? ARGB16 : RGB16;
		}
	}
	if ( isCMYK ) {
		switch ( bpc ) {
		case 8:	 return hasAlpha ? CMYKA8 : CMYK8;
		case 16: return hasAlpha ? CMYKA16 : CMYK16;
		}
	}
	throw std::runtime_error( "composePixelMode failed: internal error" );
}

