#pragma once
#include <cstdint>

typedef struct ImageCoderProperties {
public:
	bool     embeddedIccProfile    = false;  // R - true if file contained embedded ICC profile
	bool     colorSpaceIsSRGB      = false;	 // RW - true if color space is/shall be marked as SRGB 
	bool     hasColorDescription   = false;  // R - true if file contained gamma, whitepoint or chroma description
	double   gamma                 = 0.0;    // RW - file gamma
	double   whitepoint[2]         = {0};    // R - whitepoint
	double   chromaticPrimaries[6] = {0};    // R - chromatic primaries
	double   offsetInches[2]       = {0};    // R - position offset in inches
	int32_t  offsetPixels[2]       = {0};    // R - position offset in píxels
	
	// flags on how to read/write the file
	bool     applyHalftoneHintsOnRead = true;       // W - obey halft tone hints during read (TIFF)
	bool     writeSeparatedPlanes     = false;      // W - Write file in planar order ? Tiff only
	uint32_t compression              = 1;          // W - compression type, encoder specific
	double   quality                  = 0.9;        // W - quality for lossy compressions (Jpeg, Jpeg2000)
} ImageCoderProperties;


class TIFF_Compression {
public:
	TIFF_Compression() = delete;
	~TIFF_Compression() = delete;

	/** No compression */
	static constexpr uint16_t NONE = 1;
	/** CCIT modified Huffmann RLE -- MIGHT NOT WORK WITH ALL IMAGE TYPES */
	static constexpr uint16_t CCIT_RLE = 2;
	/** CCIT Fax Group 3 (T.4) -- MIGHT NOT WORK WITH ALL IMAGE TYPES */
	static constexpr uint16_t CCIT_FAX_G3 = 3;
	/** CCIT Fax Group 4 (T.6) -- MIGHT NOT WORK WITH ALL IMAGE TYPES */
	static constexpr uint16_t CCIT_FAX_G4 = 4;
	/** LZW compression */
	static constexpr uint16_t LZW = 5;
	/** JPEG DCT compression -- LOSSY !!! */
	static constexpr uint16_t JPEG = 7;
	/** Adobe Deflate */
	static constexpr uint16_t ADOBE_DEFLATE = 8;
	/** Deflate */
	static constexpr uint16_t DEFLATE=32946;
	/** CCIT modified Huffmann RLE with word alignment -- MIGHT NOT WORK WITH ALL IMAGE TYPES*/
	static constexpr uint16_t CCIT_RLE_W = 32771;
	/** Packbits (Macintosh RLE) */
	static constexpr uint16_t PACKBITS=32773;
	/** Thunderscan RLE */
	static constexpr uint16_t THUNDERSCAN_RLE=32809;
	/** LZMA2 */
	static constexpr uint16_t LZMA=34925;

};