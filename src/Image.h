/* Image.h
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
#include <string>
#include <cstdint>
#include <fstream>
#include <memory>
#include <vector>
#include "PixelMode.h"
#include "ImageCoderProperties.h"

typedef enum ImageEncoding {
	/** BMP (Windows Bitmap) File Format */
	IE_BMP,
	/** PNH (Portable Network Graphics) File Format */
	IE_PNG,
	/** GIF (Graphics Interchange Format) File Format */
	IE_GIF,	
	/** JPEG (JFIF) File Format */
	IE_JPEG,
	/** JPEG-2000 (ISO xxxx:xxxx) File Format */
	IE_JPEG2000,
	/** TIFF (Tagged Image) File Format */
	IE_TIFF,
	/** PCX (Picutre Exchange) File Format */
	IE_PCX,
	/** TGA (Targa) File Format */
	IE_TGA,

	/** USER SPECIFIC CODECS */
	IE_USER_1 = 1001,
	IE_USER_2 = 1002,
	IE_USER_3 = 1003,
	IE_USER_4 = 1004,
	IE_USER_5 = 1005,
	IE_USER_6 = 1006,
	IE_USER_7 = 1007,
	IE_USER_8 = 1008,
} ImageEncoding;

class Image
{
	friend class ImageCoder;
	friend class ColorManager;
public:
#pragma pack(push,1)
	template <typename T> struct Pixel_GRAY { T value;  };
	template <typename T> struct Pixel_AGRAY{ T alpha; T value; };
	template <typename T> struct Pixel_RGB{ T r; T g; T b; };
	template <typename T> struct Pixel_ARGB{ T alpha; T r; T g; T b; };
	template <typename T> struct Pixel_CMYK{ T c; T m; T y; T k; };
	template <typename T> struct Pixel_CMYKA{ T c; T m; T y; T k; T alpha; };
	template <typename T> struct Pixel_LAB{ T l; T a; T b; };
	template <typename T> struct Pixel_LABA{ T l; T a; T b; T alpha; };

	typedef struct Pixel_GRAY<uint8_t>   Pixel_GRAY_8;
	typedef struct Pixel_GRAY<uint16_t>  Pixel_GRAY_16;
	typedef struct Pixel_GRAY<float>     Pixel_GRAY_32;
	typedef struct Pixel_AGRAY<uint8_t>  Pixel_AGRAY_8;
	typedef struct Pixel_AGRAY<uint16_t> Pixel_AGRAY_16;
	typedef struct Pixel_AGRAY<float>    Pixel_AGRAY_32;
	typedef struct Pixel_RGB<uint8_t>    Pixel_RGB_8;
	typedef struct Pixel_RGB<uint16_t>   Pixel_RGB_16;
	typedef struct Pixel_RGB<float>      Pixel_RGB_32;
	typedef struct Pixel_ARGB<uint8_t>   Pixel_ARGB_8;
	typedef struct Pixel_ARGB<uint16_t>  Pixel_ARGB_16;
	typedef struct Pixel_ARGB<float>     Pixel_ARGB_32;
	typedef struct Pixel_CMYK<uint8_t>   Pixel_CMYK_8;
	typedef struct Pixel_CMYK<uint16_t>  Pixel_CMYK_16;
	typedef struct Pixel_CMYK<float>     Pixel_CMYK_32;
	typedef struct Pixel_CMYKA<uint8_t>  Pixel_CMYKA_8;
	typedef struct Pixel_CMYKA<uint16_t> Pixel_CMYKA_16;
	typedef struct Pixel_CMYKA<float>    Pixel_CMYKA_32;
	typedef struct Pixel_LAB<float> Pixel_LAB_32;
	typedef struct Pixel_LABA<float>    Pixel_LABA_32;
#pragma pack(pop)

	typedef struct WHITEPOINT_XYY { double x; double y; double Y; } WhitePointxyY;

protected:	
	static const std::string ColorModel_Gray;
	static const std::string ColorModel_RGB;
	static const std::string ColorModel_CMYK;
	static const std::string ColorModel_Lab;

public:
	/** Constructor.
	 * Not actually creating an image.
	 */
	Image();
	
	/** Destructor. 
	 * Freeing up all acquired resources.
	 */
	virtual ~Image();

	/** Reads image from a file.
	*
	* @param filename  Name of the file. Fully qualified or relative to working directory.
	* @param subImage  "page number" for multi-image files (such as GIF, TIFF). Will fail if sub image does not exist.
	* @param props     Optional properties object for (a) setting some behaviour, (b) gathering some values.
	*/
	void read(const std::string & filename, size_t subImage=1, ImageCoderProperties* props=nullptr );
	
	/** Reads image from a stream.
	*
	* @param stream    Stream to read from. Stream must support random access (seekg, tellg) for most image formats. Stream position will be modified during reading, not guarantees where it is when done, even less guarantees after a failure during read. Stream will not be closed.
	* @param imageName Name of the image, used for debugging output only. Could be a filename or any other identifier - or empty.
	* @param subImage  "page number" for multi-image files (such as GIF, TIFF). Will fail if sub image does not exist.
	* @param props     Optional properties object for (a) setting some behaviour, (b) gathering some values.
	*/
	void read(std::istream & stream, const std::string imageName="", size_t subImage = 1, ImageCoderProperties* props = nullptr);

	/** Reads image from a buffer.
	*
	* @param imageBuffer       buffer with image data. Will not be modified or released. 
	* @param imageBufferLength Length of buffer with image data.
	* @param imageName         Name of the image, used for debugging output only. Could be a filename or any other identifier - or empty.
	* @param subImage          "page number" for multi-image files (such as GIF, TIFF). Will fail if sub image does not exist.
	* @param props             Optional properties object for (a) setting some behaviour, (b) gathering some values.
	*/
	void read( const void* imageBuffer, size_t imageBufferSize, const std::string imageName = "", size_t subImage = 1, ImageCoderProperties* props = nullptr);
	
	/** Writes image to a file.
	*
	* @param filename       Name of the file.
	* @param ImageEncoding  Encoding (Image Format) type.
	* @param props          Optional properties object for (a) setting some behaviour, (b) gathering some values.
	*/
	void write(const std::string & filename, ImageEncoding enc, ImageCoderProperties* props=nullptr );
	
	/** Destroys the image, freeing all memory it once allocated.
	 */
	void destroy();
	
	/** Create image based on the information given.
	 * 
	 * Creates the image and allocates pixel buffer with undefined content.
	 * 
	 * @param width    Width of image (pixels per line)
	 * @param height   Height of image (number of scan lines)
	 * @param pm       Pixel Mode (Color Model, Bit Depth, Alpha)
	 * @param dpiX     Horizontal resolution
	 * @param dpiY     Vertifcal resolution
	 */
	void create(size_t width, size_t height, PixelMode pm = PixelMode::ARGB8, double dpiX = 96.0, double dpiY = 96.0);

	/** Create image with same information (width, height, PixelMode, resolution) as a source image.
	 * 
	 * Creates the image and allocates pixel buffer with undefined content.
	 * 
	 * @param other    Prototype image.
	 */
	void createFrom( const Image* other );	// empty image with same characteristics, no icc profile, no pixels (but allocated)
	
	/** Creates a copy of this image.
	 * 
	 * Creates a (deep) copy the image, copies each pixel and it's ICC Profile.
	 * 
	 * @param other    Original image.
	 */
	std::unique_ptr<Image> clone() const;

	/** Make this a copy of another image.
	 * 
	 * Copies each pixel into this object and it's ICC Profile.
	 * 
	 * @param other    Original image.
	 */
	void copy(const Image* other); // copy with same content, including icc profile.

	/** Copy alpha channel of another image into this.
	 * 
	 * Copies the Alpha channel of another image into this image. Requires this and the other image to have an alpha channel and both images to have the same size (width, height).
	 * 
	 * @param other    Original image.
	 */
	void copyAlphaFrom(const Image* other); // copy Alpha from other image. will fail if other image has different size
	
	/** Separated Image.
	 * 
	 * Separates each plane of the Image (including alpha) into an array of Gray images.
	 * Number and order of separations depend on the image format.
	 * Gray: 1 (gray)
	 * AGRAY: 2 (alpha, gray)
	 * RGB: 3 (red, green, bluee)
	 * ARGB: 4 (alpha, red, green, blue)
	 * CMYK: 4 (cyan, magenta, yellow, key)
	 * CMYKA: 5 (cyan, magenta, yellow, key, alpha)
	 * 
	 * @return  vector with the separations.
	 */
	std::vector<Image> separate() const;
	
	/** Combine Image.
	 * 
	 * Combine the planes given to this image.
	 * Number and order of separations required depend on the image format, see separate().
	 * If number of separation planes does not fit or size of images does not fit,
	 * then an eception is thrown.
	 * 
	 * @param targetPixelMode  PixelMode the image shall be created in.
	 * @param separations      vector of gray images that will make up the new image. Must all be same size and depth. Must be right number for PixelMode. Depth need to fit PixelMode.
	 */
	 void combine( PixelMode targetPixelMode, const std::vector<Image> & separations );

	/** Is this image 8 bits per channel?
	 * @return true if this image is 8 bits per channel.
	 */	
	bool is8bpc() const;
	
	/** Is this image 16 bits per channel?
	 * @return true if this image is 16 bits per channel.
	 */	
	bool is16bpc() const;	
	
	/** Is this image 32 bits per channel?
	* @return true if this image is 32 bits per channel (IEEE float)
	*/
	bool is32bpc() const;

	/** Get the PixelMode.
	 * @return The pixel mode of this image.
	 */	
	PixelMode getPixelMode() const;

	/** Get the Width.
	 * @return The width of this image (pixels per scanline).
	 */	
	uint32_t getWidth() const;

	/** Get the Height.
	 * @return The height of this image (number of scanlines).
	 */	
	uint32_t getHeight() const;

	/** Get the number of pixels.
	 * @return The number of pixels in this image (width * height).
	 */	
	uint32_t getNumPixels() const;

	/** Get the Stride of the image.
	 * @return The number bytes per line, including padding.
	 */	
	uint32_t getStride() const;
	
	/** Get Bytes per Channel.
	* @return The number of bytes per channel.
	*/
	uint32_t getChannelSize() const;

	/** Get Bytes per Pixel.
	 * @return The number of bytes per pixel, including padding (typically, pixels are not padded).
	 */	
	uint32_t getPixelSize() const;
	
	/** Get the Horizontal resolution.
	 * @return Horizontal resolution of the image (dpi).
	 */	
	double getResolutionX() const;

	/** Get the Vertical resolution.
	 * @return Vertical resolution of the image (dpi).
	 */	
	double getResolutionY() const;

	/** Set the resolution.
	 * @param dpiX Horizontal resolution in dpi.
	 * @param dpiY Vertical resolution in dpi.
	 */	
	void setResolution(double dpiX, double dpiY);

	/** Is the Color Model CMYK?
	 * @return true if image is encoded in CMYK color model.
	 */	
	bool isCMYK() const;

	/** Is the Color Model RGB?
	 * @return true if image is encoded in RGB color model.
	 */	
	bool isRGB() const;

	/** Is the Color Model Gray?
	 * @return true if image is encoded in Gray color model (might also be bilevel).
	 */	
	bool isGray() const;

	/** Is the Color Model L*a*b*? 
	 * @return true if image is encoded in L*a*b* color model.
	 */
	bool isLab() const;

	/** Does the image have an alpha channel?
	 * @return true if image has an alpha channel.
	 */	
	bool hasAlpha() const;

	/** Does the image have an ICC profile ?
	 * @return true if image has an ICC profile.
	 */
	bool hasIccProfile() const;

	/** Get the icc profile.
	 * @return Unmodifiable string with icc profile content. Don't mess with it!
	 */
	const std::string & getIccProfileContent() const;

	/** Set the icc profile content. You better make sure it's a valid profile!
	 */
	void setIccProfile(const std::string iccProfile);

	/** Set the icc profile content. You better make sure it's a valid profile!
	*/
	void setIccProfile(const void* buffer, size_t len );

	/** Get the white point for a LAB image
	 */
	const WhitePointxyY* getLabWhitePoint() const;
	void setLabWhitePoint(double x, double y, double Y);

private:
	// this structure is similar (!!!) to the lplImage structure in openCV and Intel IPL for quick conversion... but certainly, not equal !
	typedef struct Header {
		size_t      nSize = sizeof(Header); // sizeof struct - not that this may differ a lot depending on the size of std::string! This is not a good model, yet...
		int         ID             = 0; // Version, always equals 0
		int         nChannels      = 4; // Number of channels. Default 4 (ARGB)
		int         alphaChannel   = 0; // Alpha channel; -1: no alpha; >=0: index of alpha channel. max value: nChannels-1
		int         depth          = 8; // Bits per channel, 1, 8, 16 (UINT) or 32 (float) or 64 (double)                         -- NOTE THAT WE CURRENTLY ONLY SUPPORT 8 and 16 and 32! Not 1 or 64!
		std::string colorModel     = "RGB";	// just for information, not really used
		std::string channelSeq     = "ARGB"; // just for information, not really used 
		int         dataOrder      = 0; // data order pixel (0:chunks (e.g. RGBRGBRGB...); 1: planes(e.g. RRRR...GGGG...BBBB...)) -- NOTE THAT WE ONLY SUPPORT 0
		int         origin         = 0; // top left (1: bottom left);		   	                                                 -- NOTE THAT WE ONLY SUPPORT 0
		int         pixelAlign     = 1; // pixel alignment
		int         lineAlign      = 4; // Lines are aligned ...
		int         bytesPerPixel  = 4; // bytes per Pixel (all color samples! Depth/8*nChannels) - EXCLUDING PADDING
		size_t      width     = 0;
		size_t      height    = 0;
		size_t      widthStep = 0;  // "size of aligned line in bytes" (stride)
		size_t      imageSize = 0;  // size of data, including padding. Total size of array in pixels.
	} Header;

protected:
	// The actual data of an image
	PixelMode     pixelMode;	// how are the pixels internally stored ?
	Header        header;
	std::string   iccProfile;
	double        dpiX;
	double        dpiY;
	WhitePointxyY whitePoint;	// only valid for lab images - default D50
	std::unique_ptr<uint8_t[]> pixels; // carefull... lines have a stride! see: header.widthStep ! 
	
private:
	// temporary values, only true during read().
	size_t subImageToRead;	// valid during read - index of image in file to read (1 .. n). 0: read all (not yet handled)
	size_t subImageRead; // valid after read - index of image in file that was (last) read. (1 .. n)

protected:
	bool imageReadCallback( size_t subImage ); // will be called if encoder read an image from the file and/or is done.
	void createImpliciteProfile( const ImageCoderProperties * props ); 

public:
	/** Get a scan line.
	 * 
	 * @param line   0-based index of the line, must be 0 .. getHeight()-1
	 * @return Pointer to the first byte of the first pixel of a line. Each line is getStride() bytes long and has getWidth() pixels.
	 */
	uint8_t* getLine( size_t line );
	const uint8_t*  getLineConst(size_t line) const;

	/** Get a pixel.
	 * 
	 * @param line     0-based index of the line, must be 0 .. getHeight()-1
	 * @param column   0-based index of the pixel within the line (x position), must be 0 .. getWidth()-1
	 * @return Pointer to the beginning of the pixel information. The pixel is getPixelSize() bytes long.
	 */
	uint8_t* getPixel( size_t line, size_t column );
	const uint8_t* getPixelConst(size_t line, size_t column) const;

	/** Get a pixel.
	 * 
	 * @param line     Pointer to the first byte of the first pixel of a line (as returned by getLine()).
	 * @param column   0-based index of the pixel within the line (x position), must be 0 .. getWidth()-1
	 * @return Pointer to the beginning of the pixel information. The pixel is getPixelSize() bytes long.
	 */
	uint8_t* getPixel( void* line, size_t column );
	const uint8_t* getPixelConst(const void* line, size_t column) const;

public:
	// modification methods
	/** Fills image or region within the image with a given color.
	 * 
	 * @param pixel  Pointer to a pixel value to applied to each pixel. Color Model, Depth and alpha channel must be formatted correctly for current image type. Objet pointed to must be getPixelSize() bytes.
	 * @param x      Origin column (top left), must be 0 .. getWidth()-1.
	 * @param y      Origin line (top left), must be 0 .. getHeight()-1.
	 * @param w      Width of region to fill. If 0 or > (getWidth()-x), the complete image (from origin) will be filled.
	 * @param h      Height of region to fill. If 0 or > (getHeight()-y), the complete image (from origin) will be filled.
	 */
	void fill( const void* pixel, size_t x=0, size_t y=0, size_t w=0, size_t h=0 );

	/** Converts image to 8 bits per channel.
	 * 
	 * If image already has 8bpc, image will be copied.
     *
	 * @param out  Target image. If nullptr, conversion will be done in-place.
	 */
	void convertTo8bpc( Image* out=nullptr );

	/** Converts image to 16 bits per channel.
	 * 
	 * If image already has 16bpc, image will be copied.
	 * 
	 * @param out  Target image. If nullptr, conversion will be done in-place.
	 */
	void convertTo16bpc(Image* out = nullptr);

	/** Converts image to 32 bits per channel.
	*
	* If image already has 32bpc, image will be copied.
	*
	* @param out  Target image. If nullptr, conversion will be done in-place.
	*/
	void convertTo32bpc(Image* out = nullptr);
	
	/** Remove the alpha channel from the image.
	 * 
	 * Will fail if image does not have an alpha channel.
	 * 
	 * @param multyplyOntoColors  Will "pre-multiply" color values. True: Behaviour as if the image would be composited onto a white background. False: Don't touch colors, Just drop the transparency.
	 * @param out                 Target image. If nullptr, alpha channel will be removed in-place from current image.
	 */
	void removeAlphaChannel(bool multiplyOntoColors = false, Image* out = nullptr);

	/** Add an alpha channel to the image.
	 * 
	 * Will fail if image already has an alpha channel.
	 * 
	 * @param setAllTransparent  transparency value (0 to 1) for each pixel.
	 * @param out                Target image. If nullptr, alpha channel will be added in-place to current image.
	 */
	void addAlphaChannel(double transparency = 0.0, Image* out = nullptr);
	
	
	enum class FlipDirection : int { Horizontal=1, Vertical=2, SwapXY=4 };
	
	/** Flip the image... can be combined to a rotation.
	 * 
	 * @param flipAxis  Select the flip axis or rotation (which is a series of two flips).
	 * @param out       Target image. If nullptr, the image will be flipped in-place to current image.
	 */
	void flip( FlipDirection flipAxis, Image* out=nullptr );
	
#if 0 // this stuff isn't implemented yet and should move out to separate filter classes (perhaps including the "flip")
	  
	/** Invert the image.
	 * 
	 * @param channel   Channel to apply the change on. -1 means all but alpha channel.
	 * @param out       Target image. If nullptr, the image will be flipped in-place to current image. 
	 */
	void invert( int32_t channel=-1, Image* out=nullptr );

	/** Levels filter.
	 * 
	 * @param inBlack   Black input level. 0 always means black. Range: 0 ... 2^depth-2. Neutral: 0
	 * @param inWhite   White input level. 0 always means black. Range: inBlack+1 ... 2^depth-1. Neutral: 2^depth-1
	 * @param gamma     Gamma correction value; Range: 0.05 - 20.0; Neutral: 1.0. 
	 * @param outBlack  Black output level. 0 always means black. Range: 0 ... 2^depth-2. Neutral: 0.
	 * @param outWhite  White output level. 0 always means black. Range: outBlack+1 ... 2^depth-1. Neutral: 2^depth-1
	 * @param channel   Channel to apply the change on. -1 means all but alpha channel. For L*a*b* images, channel -1 means L* only.
	 * @param out       Target image. If nullptr, the image will be flipped in-place to current image.
	 */
	void levels( uint32_t inBlack=0, uint32_t inWhite=255, double gamma=1.0, uint32_t outBlack=0, uint32_t outWhite=255, int32_t channel=-1, Image* out=nullptr );

	/** Levels filter.
	*
	* @param inBlack   Black input level. 0 always means black. Range: 0.0 ... 1.0. Neutral: 0.0
	* @param inWhite   White input level. 0 always means black. Range: >outBlack ... 1.0. Neutral: 1.0
	* @param gamma     Gamma correction value; Range: 0.05 - 20.0; Neutral: 1.0.
	* @param outBlack  Black output level. 0 always means black. Range: 0.0 ... 1.0. Neutral: 0.0
	* @param outWhite  White output level. 0 always means black. Range: >outBlack ... 1.0. Neutral: 1.0
	* @param channel   Channel to apply the change on. -1 means all but alpha channel. For L*a*b* images, channel -1 means L* only.
	* @param out       Target image. If nullptr, the image will be flipped in-place to current image.
	*/
	void levels( double inBlack=0.0, double inWhite=1.0, double gamma= 1.0, double outBlack=0.0, double outWhite=1.0, int32_t channel = -1, Image* out = nullptr);

	/** Brightness & Contrast.
	 * 
	 * @param brightness  Amount of brightness Change. Range: -(2^depth-1) ... +(2^depth-1) [-255...+255 for 8 bits]. Neutral: 0
	 * @param contrast    Amount of contrast Change. Range: -((2^(depth-1)-1) ... +((2^(depth-1))-1) [-127...+127 for 8 bits]. Neutral: 0.
	 * @param channel     Channel to apply the change on. -1 means all but alpha channel. For L*a*b* images, channel -1 means L* only.
	 * @param out         Target image. If nullptr, the image will be flipped in-place to current image. 
	 */
	void brightnessContrast( int32_t brightness=0, int32_t contrast=0, int32_t channel=-1, Image* out=nullptr );
	/** Brightness & Contrast.

	*
	* @param brightness  Amount of brightness Change. Range: -1.0 to +1.0 [-255...+255 for 8 bits]. Neutral: 0
	* @param contrast    Amount of contrast Change. Range: -1.0 to +1.0. Neutral: 0.
	* @param channel     Channel to apply the change on. -1 means all but alpha channel. For L*a*b* images, channel -1 means L* only.
	* @param out         Target image. If nullptr, the image will be flipped in-place to current image.
	*/

	void brightnessContrast( double brightness=0.0, double contrast=0.0, int32_t channel = -1, Image* out = nullptr);

	/** Apply table. Only works for INT based pixel values, not float based.
	 * 
	 * @param table    Table for the selected channel. size must be 2^depth.
	 * @param channel  Channel to apply the table on. -1 means all but alpha channel. For L*a*b* images, channel -1 means L* only.
	 * @param out         Target image. If nullptr, the image will be flipped in-place to current image. 
	 * @note: will fail on float or double based images.
	 */ 
	void applyTable( std::vector<uint16_t> table, int32_t channel=-1, Image* out=nullptr );
#endif
	/** Calculate a hash for the image. The purpose is to test if two images are equal by content.
	 * Uses a 64bit Fowler/Vo/Noll 1a hash.
	 * The hash is calculated over header, pixelMode, DPI, (optionally) ICC Profile and Pixel data.
	 * 
	 * @return 64 bit hash value
	 */
	uint64_t calcHash(bool includeIccProfile = true) const;

	static uint64_t calcHash(const std::string & test);
};

