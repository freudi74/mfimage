/* ColorManager.cpp
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
#include "ColorManager.h"
#include <math.h>

/*static*/ const cmsCIExyY ColorManager::WHITE_POINT_D50 = { 0.34567, 0.35850, 1.0 };
/*static*/ const cmsCIExyY ColorManager::WHITE_POINT_D55 = { 0.33242, 0.34743, 1.0 };
/*static*/ const cmsCIExyY ColorManager::WHITE_POINT_D65 = { 0.31271, 0.32902, 1.0 };
/*static*/ const cmsCIExyY ColorManager::WHITE_POINT_D75 = { 0.29902, 0.31485, 1.0 };


ColorManager::ColorManager(void* pluginCollection)
	: _lastErrorCode(0)
{
	_ctx = cmsCreateContext(pluginCollection, this);
	if (_ctx == nullptr)
	{
		throw std::runtime_error("Failed to create ColorManagementSystem context.");
	}
	cmsSetLogErrorHandlerTHR(_ctx, errorHandlerCallback);
}


ColorManager::~ColorManager()
{
	if (_ctx != nullptr)
	{
		cmsDeleteContext(_ctx);
		_ctx = nullptr;
	}
}

cmsHPROFILE ColorManager::readProfileFromFile(const std::string & filename)
{
	cmsHPROFILE hp = cmsOpenProfileFromFileTHR(_ctx, filename.c_str(), "r");
	if (hp == nullptr)
		throw std::runtime_error("failed to read profile " + filename + formatError() );
	return hp;
}

cmsHPROFILE ColorManager::readProfileFromMem(const void* src, uint32_t size)
{
	cmsHPROFILE hp = cmsOpenProfileFromMemTHR(_ctx, src, size );
	if (hp == nullptr)
		throw std::runtime_error("failed to read profile from memory" + formatError());
	return hp;
}

cmsHPROFILE ColorManager::readProfileFromImage(const Image* img, cmsHPROFILE defaultProfile)
{
	if (img->iccProfile.empty())
		return defaultProfile;
	cmsHPROFILE hp = readProfileFromMem(img->iccProfile.c_str(), img->iccProfile.size());
//	cmsSaveProfileToFile(hp, "C:\\svn\\testIn.icc");
	return hp;
}

void ColorManager::embeddProfile(Image* img, cmsHPROFILE profile)
{
	if (profile == NULL)
		throw std::runtime_error("profile invalid");
	
	// get size of profile
	uint32_t profileLength;
	if ( ! cmsSaveProfileToMem(profile, nullptr, &profileLength) )
		throw std::runtime_error("failed to embed profile" + formatError());
	// prepare a buffer
	std::unique_ptr<char[]> pbuffer(new char[profileLength]);
	// put things in buffer
	if (!cmsSaveProfileToMem(profile, pbuffer.get(), &profileLength))
		throw std::runtime_error("failed to embed profile" + formatError());
	// store in image
//	cmsSaveProfileToFile(profile, "C:\\svn\\test.icc");
	img->iccProfile.assign(pbuffer.get(), profileLength);
}


cmsHPROFILE ColorManager::createRgbProfile(const cmsCIExyY* whitePoint, const cmsCIExyYTRIPLE* primaries, double gamma )
{
	cmsToneCurve* gammaToneCurve = cmsBuildGamma(_ctx, gamma);
	cmsToneCurve* rgbToneCurves[3] = { gammaToneCurve, gammaToneCurve, gammaToneCurve };
	cmsHPROFILE hp = cmsCreateRGBProfileTHR(_ctx, whitePoint, primaries, rgbToneCurves);
	cmsFreeToneCurve( gammaToneCurve );
	if (hp == nullptr)
		throw std::runtime_error("failed to create RGB profile" + formatError());
	return hp;
}
cmsHPROFILE ColorManager::createGrayProfile(const cmsCIExyY* whitePoint, double gamma)
{
	cmsToneCurve* gammaToneCurve = cmsBuildGamma(_ctx, gamma);
//	cmsToneCurve* gammaToneCurve2 = buildGamma(gamma, 32768);
	cmsHPROFILE hp = cmsCreateGrayProfileTHR(_ctx, whitePoint, gammaToneCurve);
	cmsFreeToneCurve( gammaToneCurve );
	if (hp == nullptr)
		throw std::runtime_error("failed to create Gray profile" + formatError());
	return hp;
}
cmsHPROFILE ColorManager::createSrgbProfile()
{
	cmsHPROFILE hp = 	(_ctx);
	if (hp == nullptr)
		throw std::runtime_error("failed to create sRGB profile" + formatError());
	return hp;
}
cmsHPROFILE ColorManager::createNullProfile()
{
	cmsHPROFILE hp = cmsCreateNULLProfileTHR(_ctx);
	if (hp == nullptr)
		throw std::runtime_error("failed to create NULL profile" + formatError());
	return hp;
}
void ColorManager::closeProfile(cmsHPROFILE profile)
{
	if ( ! cmsCloseProfile(profile) )
		throw std::runtime_error("failed to close profile" + formatError());
}

cmsHTRANSFORM ColorManager::createTransform(cmsHPROFILE pin, cmsHPROFILE pout, uint32_t intent, uint32_t inputFmt, uint32_t outputFmt, uint32_t flags)
{
	cmsHPROFILE ht = cmsCreateTransformTHR(_ctx, pin, inputFmt, pout, outputFmt, intent, flags );
	if (ht == nullptr)
		throw std::runtime_error("failed to create Transformation" + formatError());
	return ht;
}

void ColorManager::closeTransform(cmsHTRANSFORM trans)
{
	cmsDeleteTransform(trans);
}

void ColorManager::errorHandlerCallback(cmsContext ContextID, cmsUInt32Number ErrorCode, const char *Text)
{
	if (ContextID == NULL)
		return;
	ColorManager* self = reinterpret_cast<ColorManager*>( cmsGetContextUserData(ContextID) );
	if (self)
		self->errorHandler(ErrorCode, Text);
}

void ColorManager::errorHandler(uint32_t errorCode, const char* text)
{
	_lastErrorCode = errorCode;
	_lastErrorText.assign(text);
}

std::string ColorManager::formatError()
{
	return " - Error Code " + std::to_string(_lastErrorCode) + " message: '" + _lastErrorText + "'";
}

// does not help ! Wait for lcms 2.8 ? Marti mentioned, it might got improved.
cmsToneCurve* ColorManager::buildGamma(double gamma, size_t entries)
{
/*	cmsUInt16Number* values = new cmsUInt16Number[entries];
	
	for ( size_t i=0; i<entries; i++ )
	{
		double v= static_cast<double>(i)/static_cast<double>(entries-1);
		values[i] = static_cast<cmsUInt16Number>( ( pow( v, gamma ) * 65535.0 ) );
	}
	cmsToneCurve* myCurve = cmsBuildTabulatedToneCurve16(_ctx, entries, values );
*/
	cmsFloat32Number* values = new cmsFloat32Number[entries];
	for ( size_t i=0; i<entries; i++ )
	{
		double v= static_cast<double>(i)/static_cast<double>(entries-1);
		values[i] = static_cast<cmsFloat32Number>( ( pow( v, gamma ) ) );
	}
	cmsToneCurve* myCurve = cmsBuildTabulatedToneCurveFloat(_ctx, entries, values );
	std::string error = formatError();
	delete[] values;
	return myCurve;
}
