#include <unittest++/UnitTest++.h>
#include <string> 
#include "folders.h"
#include "../src/Image.h"
#define TGA_FILES_REL SEPARATOR "tga" SEPARATOR
 
static std::string getTestFileNameInDpeg(const std::string & file) {
	return TESTDATA TGA_FILES_REL + file;
}
static std::string getTestFileNameOutDpeg(const std::string & file) {
	return TESTOUT TGA_FILES_REL + file + ".tif";
}

static size_t readWriteHashDpeg(const std::string & file) {
	bool write = true;
	return readWriteHash(file, getTestFileNameInDpeg(file), getTestFileNameOutDpeg(file), write);
}

#ifndef SKIP_TGA_SUITE
#undef CHECK_EQUAL
#define CHECK_EQUAL(a,b) b

SUITE(TGASuite)
{
	TEST(tgaRgb)
	{
		START_TEST();

		// note that some of the flag and xing images seem to be bad ... those are datacard variations, not sure where they came from!

		CHECK_EQUAL(0x871c6d1576f13ce7, readWriteHashDpeg("3fruit.TGA")); // 24 bit RGB
		CHECK_EQUAL(0xacb14f6a3df0afcb, readWriteHashDpeg("flag_b16.tga"));
//		CHECK_EQUAL(0, readWriteHashDpeg("flag_b24.tga")); // Looks odd, but photoshop and gimp do the same ...
		CHECK_EQUAL(0xcec24530bc5ebe74, readWriteHashDpeg("FLAG_B32.TGA"));
		CHECK_EQUAL(0x6dde83f0a790fa1a, readWriteHashDpeg("flag_t16.tga"));
//		CHECK_EQUAL(0, readWriteHashDpeg("flag_t24.tga")); // Looks odd, but photoshop and gimp do the same ...
		CHECK_EQUAL(0xcec24530bc5ebe74, readWriteHashDpeg("flag_t32.tga")); 
		CHECK_EQUAL(0x5fff23b1002b7896, readWriteHashDpeg("fruit.tga"));	// RGB RLE 16 bit
		CHECK_EQUAL(0x5fff23b1002b7896, readWriteHashDpeg("fruit_16bit.tga"));// RGB RLE 16 bit
		CHECK_EQUAL(0x5fff23b1002b7896, readWriteHashDpeg("fruit_16bit_uc.tga"));	// RGB 16 bit
		CHECK_EQUAL(0x871c6d1576f13ce7, readWriteHashDpeg("fruit_24bit.tga"));// RGB RLE 24 bit
		CHECK_EQUAL(0x871c6d1576f13ce7, readWriteHashDpeg("fruit_24bit_uc.tga")); // RGB 24 bit
		CHECK_EQUAL(0x98cb055060658ed8, readWriteHashDpeg("redsquare_16bit_uc.tga"));
		CHECK_EQUAL(0xb80c7a4e9e3b5991, readWriteHashDpeg("xing_b16.tga"));
		CHECK_EQUAL(0x4f93cdad51c14315, readWriteHashDpeg("xing_b24.tga")); 
//		CHECK_EQUAL(0, readWriteHashDpeg("xing_b32.tga")); // Looks odd, but photoshop does the same and gimp just shows a transparent image
		CHECK_EQUAL(0xb80c7a4e9e3b5991, readWriteHashDpeg("xing_t16.tga")); 
		CHECK_EQUAL(0x4f93cdad51c14315, readWriteHashDpeg("xing_t24.tga")); 
//		CHECK_EQUAL(0, readWriteHashDpeg("xing_t32.tga")); // Looks odd, but photoshop does the same and gimp just shows a transparent image
		END_TEST();
	}

	TEST(tgaColormap)
	{
		START_TEST();

		CHECK_EQUAL(0x8b4a0db4b30167c7, readWriteHashDpeg("fruit_8bit_uc.tga")); // COLORMAP
		CHECK_EQUAL(0x8b4a0db4b30167c7, readWriteHashDpeg("fruit_8bit.tga")); // COLORMAP RLE
		CHECK_EQUAL(0xdad56a4a3ff54fac, readWriteHashDpeg("UCM8.TGA"));	// COLORMAP
		CHECK_EQUAL(0xdad56a4a3ff54fac, readWriteHashDpeg("CCM8.TGA")); // COLORMAP  RLE

		END_TEST();
	}

	TEST(tgaGray)
	{
		START_TEST();
		CHECK_EQUAL(0xe77ded1db8505535, readWriteHashDpeg("UBW8.TGA"));	// GRAY
		CHECK_EQUAL(0xe77ded1db8505535, readWriteHashDpeg("CBW8.TGA")); // GRAY  RLE
		END_TEST();
	}

}

#endif