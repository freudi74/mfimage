/* ColorManager.h
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
#include <lcms2.h>
#include <stdexcept>
#include <cstdint>
#include "Image.h"

class ColorManager
{
public:
	static const cmsCIExyY WHITE_POINT_D50;
	static const cmsCIExyY WHITE_POINT_D55;
	static const cmsCIExyY WHITE_POINT_D65;
	static const cmsCIExyY WHITE_POINT_D75;

	ColorManager( void* pluginCollection = nullptr );
	virtual ~ColorManager();

	cmsHPROFILE readProfileFromFile(const std::string & filename);
	cmsHPROFILE readProfileFromMem(const void* src, uint32_t size);
	// embedded profiles
	cmsHPROFILE readProfileFromImage(const Image* img, cmsHPROFILE defaultProfile=nullptr );
	void embeddProfile(Image* img, cmsHPROFILE profile);
	
	cmsHPROFILE createRgbProfile(const cmsCIExyY* whitePoint, const cmsCIExyYTRIPLE* primaries, double gamma);
	cmsHPROFILE createSrgbProfile();
	cmsHPROFILE createGrayProfile(const cmsCIExyY* whitePoint, double gamma);
	cmsHPROFILE createNullProfile();
	void closeProfile(cmsHPROFILE profile);

	cmsHTRANSFORM createTransform(cmsHPROFILE pin, cmsHPROFILE pout, uint32_t intent, uint32_t inputFmt, uint32_t outputFmt, uint32_t flags = 0);
	void closeTransform(cmsHTRANSFORM trans);
	cmsToneCurve* buildGamma( double gamma, size_t entries );

protected:
	static void errorHandlerCallback( cmsContext ContextID, cmsUInt32Number ErrorCode, const char *Text);
	void errorHandler(uint32_t errorCode, const char* text);
	std::string formatError();
	cmsContext _ctx = nullptr;
	uint32_t    _lastErrorCode;
	std::string _lastErrorText;
};

// RAII helpers
class RAIIcmsHProfile {
public:
	RAIIcmsHProfile( cmsHPROFILE aProfile ) { hProfile = aProfile; }
	~RAIIcmsHProfile() { /*cmsCloseProfile(hProfile); hProfile = nullptr;*/ }
// forbidd assigning something
	RAIIcmsHProfile( const RAIIcmsHProfile& ) = delete;
	RAIIcmsHProfile& operator=(const RAIIcmsHProfile&) = delete;
	inline operator cmsHPROFILE() { return hProfile; }
private:
	cmsHPROFILE hProfile;
};
// RAII helpers
class RAIIcmsHTransform {
public:
	RAIIcmsHTransform( cmsHTRANSFORM aTransform ) { hTransform = aTransform; }
	~RAIIcmsHTransform() { cmsDeleteTransform(hTransform); }
	inline operator cmsHTRANSFORM() { return hTransform; }
// forbidd assigning something
	RAIIcmsHTransform( const RAIIcmsHTransform& ) = delete;
	RAIIcmsHTransform& operator=(const RAIIcmsHTransform&) = delete;
private:
	cmsHTRANSFORM hTransform;
};
