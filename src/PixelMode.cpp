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
	case GRAY8:   _isCMYK = false; _isRGB = false; _isGray = true;  _isLab = false; _hasAlpha = false; _numChannels=1; _bitsPerChannel = 8;  break;
	case GRAY16:  _isCMYK = false; _isRGB = false; _isGray = true;  _isLab = false; _hasAlpha = false; _numChannels=1; _bitsPerChannel = 16; break;
	case GRAY32:  _isCMYK = false; _isRGB = false; _isGray = true;  _isLab = false; _hasAlpha = false; _numChannels=1; _bitsPerChannel = 32; break;
	case AGRAY8:  _isCMYK = false; _isRGB = false; _isGray = true;  _isLab = false; _hasAlpha = true;  _numChannels=2; _bitsPerChannel = 8;  break;
	case AGRAY16: _isCMYK = false; _isRGB = false; _isGray = true;  _isLab = false; _hasAlpha = true;  _numChannels=2; _bitsPerChannel = 16; break;
	case AGRAY32: _isCMYK = false; _isRGB = false; _isGray = true;  _isLab = false; _hasAlpha = true;  _numChannels=2; _bitsPerChannel = 32; break;
	case RGB8:    _isCMYK = false; _isRGB = true;  _isGray = false; _isLab = false; _hasAlpha = false; _numChannels=3; _bitsPerChannel = 8;  break;
	case RGB16:   _isCMYK = false; _isRGB = true;  _isGray = false; _isLab = false; _hasAlpha = false; _numChannels=3; _bitsPerChannel = 16; break;
	case RGB32:   _isCMYK = false; _isRGB = true;  _isGray = false; _isLab = false; _hasAlpha = false; _numChannels=3; _bitsPerChannel = 32; break;
	case ARGB8:   _isCMYK = false; _isRGB = true;  _isGray = false; _isLab = false; _hasAlpha = true;  _numChannels=4; _bitsPerChannel = 8;  break;
	case ARGB16:  _isCMYK = false; _isRGB = true;  _isGray = false; _isLab = false; _hasAlpha = true;  _numChannels=4; _bitsPerChannel = 16; break;
	case ARGB32:  _isCMYK = false; _isRGB = true;  _isGray = false; _isLab = false; _hasAlpha = true;  _numChannels=4; _bitsPerChannel = 32; break;
	case CMYK8:   _isCMYK = true;  _isRGB = false; _isGray = false; _isLab = false; _hasAlpha = false; _numChannels=4; _bitsPerChannel = 8;  break;
	case CMYK16:  _isCMYK = true;  _isRGB = false; _isGray = false; _isLab = false; _hasAlpha = false; _numChannels=4; _bitsPerChannel = 16; break;
	case CMYK32:  _isCMYK = true;  _isRGB = false; _isGray = false; _isLab = false; _hasAlpha = false; _numChannels=4; _bitsPerChannel = 32; break;
	case CMYKA8:  _isCMYK = true;  _isRGB = false; _isGray = false; _isLab = false; _hasAlpha = true;  _numChannels=5; _bitsPerChannel = 8;  break;
	case CMYKA16: _isCMYK = true;  _isRGB = false; _isGray = false; _isLab = false; _hasAlpha = true;  _numChannels=5; _bitsPerChannel = 16; break;
	case CMYKA32: _isCMYK = true;  _isRGB = false; _isGray = false; _isLab = false; _hasAlpha = true;  _numChannels=5; _bitsPerChannel = 32; break;
	case LAB32:   _isCMYK = false; _isRGB = false; _isGray = false; _isLab = true;  _hasAlpha = false; _numChannels=3; _bitsPerChannel = 32; break;
	case LABA32:  _isCMYK = false; _isRGB = false; _isGray = false; _isLab = true;  _hasAlpha = false; _numChannels=4; _bitsPerChannel = 32; break;
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
	case GRAY32:  return TYPE_GRAY_FLT;
	case AGRAY8:  return TYPE_GRAYA_8|SWAPFIRST_SH(1);
	case AGRAY16: return TYPE_GRAYA_16|SWAPFIRST_SH(1);
	case AGRAY32: return TYPE_GRAY_FLT|EXTRA_SH(1)|SWAPFIRST_SH(1);
	case RGB8:    return TYPE_RGB_8;
	case RGB16:   return TYPE_RGB_16;
	case RGB32:   return TYPE_RGB_FLT;
	case ARGB8:   return TYPE_ARGB_8;
	case ARGB16:  return TYPE_ARGB_16;
	case ARGB32:  return TYPE_ARGB_FLT;
	case CMYK8:   return TYPE_CMYK_8;
	case CMYK16:  return TYPE_CMYK_16;
	case CMYK32:  return TYPE_CMYK_FLT;
	case CMYKA8:  return TYPE_CMYKA_8;
	case CMYKA16: return TYPE_CMYK_16|EXTRA_SH(1);
	case CMYKA32: return TYPE_CMYK_FLT|EXTRA_SH(1);
	case LAB32:   return TYPE_Lab_FLT;
	case LABA32:  return TYPE_LabA_FLT;
		// no default to get compiler warning
	}
	return TYPE_RGB_8;
}

PixelMode PixelMode::create(const std::string & colorModel, bool hasAlpha, uint32_t bitsPerChannel)
{
	return compose(colorModel, hasAlpha, bitsPerChannel);

}

PixelMode PixelMode::create(bool isCMYK, bool isRGB, bool isGray, bool isLab, bool hasAlpha, uint32_t bitsPerChannel)
{
	return compose(isCMYK, isRGB, isGray, isLab, hasAlpha, bitsPerChannel);
}

PixelMode::PixelModeEnum PixelMode::compose(const std::string & colorModel, bool hasAlpha, uint32_t bitsPerChannel)
{
	bool isRGB = false;
	bool isCMYK = false;
	bool isGray = false;
	bool isLab = false;
	if (colorModel == "RGB")
		isRGB = true;
	else if (colorModel == "CMYK")
		isCMYK = true;
	else if (colorModel == "GRAY" || colorModel == "Gray" )
		isGray = true;
	else if (colorModel == "LAB" || colorModel == "Lab")
		isLab = true;
	else
		throw std::runtime_error("Unknown/Unsupported color model: " + colorModel);
	return compose(isCMYK, isRGB, isGray, isLab, hasAlpha, bitsPerChannel);
}

PixelMode::PixelModeEnum PixelMode::compose(bool isCMYK, bool isRGB, bool isGray, bool isLab, bool hasAlpha, uint32_t bitsPerChannel)
{
	int colorModelsSet = 0;
	if (isCMYK) colorModelsSet++;
	if (isRGB) colorModelsSet++;
	if (isGray) colorModelsSet++;
	if (isLab) colorModelsSet++;
	if (colorModelsSet != 1)
		throw std::runtime_error( "composePixelMode failed: more or less than one color model set" );
	if (bitsPerChannel != 8 && bitsPerChannel != 16 && bitsPerChannel != 32)
		throw std::runtime_error( "composePixelMode failed: unsupported bitsPerChannel" );
	
	if ( isGray ) 
	{
		switch (bitsPerChannel) 
		{
		case 8:	 return hasAlpha ? AGRAY8 : GRAY8;
		case 16: return hasAlpha ? AGRAY16 : GRAY16;
		case 32: return hasAlpha ? AGRAY32 : GRAY32;
		}
	}
	if ( isRGB ) 
	{
		switch (bitsPerChannel)
		{
		case 8:	 return hasAlpha ? ARGB8 : RGB8;
		case 16: return hasAlpha ? ARGB16 : RGB16;
		case 32: return hasAlpha ? ARGB32 : RGB32;
		}
	}
	if ( isCMYK ) 
	{
		switch (bitsPerChannel)
		{
		case 8:	 return hasAlpha ? CMYKA8 : CMYK8;
		case 16: return hasAlpha ? CMYKA16 : CMYK16;
		case 32: return hasAlpha ? CMYKA32 : CMYK32;
		}
	}
	if ( isLab ) 
	{
		switch (bitsPerChannel)
		{
		case 8:	 throw std::runtime_error("composePixelMode failed: unsupported bitsPerChannel for L*a*b* color model");
		case 16: throw std::runtime_error("composePixelMode failed: unsupported bitsPerChannel for L*a*b* color model");
		case 32: return hasAlpha ? LABA32 : LAB32;
		}
	}
	throw std::runtime_error( "composePixelMode failed: internal error" );
}

