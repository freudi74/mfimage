#include <UnitTest++/UnitTest++.h>
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

SUITE(TGASuite)
{
	TEST(tgaRgb)
	{
		START_TEST();
		// note that some of the flag and xing images seem to be bad ... those are datacard variations, not sure where they came from!
		CHECK_EQUAL(0xf45e775d03fde5b2, readWriteHashDpeg("3fruit.TGA")); // 24 bit RGB
		CHECK_EQUAL(0xb300b0250b333691, readWriteHashDpeg("flag_b16.tga"));
//		CHECK_EQUAL(0, readWriteHashDpeg("flag_b24.tga")); // Looks odd, but photoshop and gimp do the same ...
		CHECK_EQUAL(0xb864a4e5249c291e, readWriteHashDpeg("FLAG_B32.TGA"));
		CHECK_EQUAL(0x0952ea23af118d54, readWriteHashDpeg("flag_t16.tga"));
//		CHECK_EQUAL(0, readWriteHashDpeg("flag_t24.tga")); // Looks odd, but photoshop and gimp do the same ...
		CHECK_EQUAL(0xb864a4e5249c291e, readWriteHashDpeg("flag_t32.tga")); 
		CHECK_EQUAL(0xcc89c4cd6e3e9fbb, readWriteHashDpeg("fruit.tga"));	// RGB RLE 16 bit
		CHECK_EQUAL(0xcc89c4cd6e3e9fbb, readWriteHashDpeg("fruit_16bit.tga"));// RGB RLE 16 bit
		CHECK_EQUAL(0xcc89c4cd6e3e9fbb, readWriteHashDpeg("fruit_16bit_uc.tga"));	// RGB 16 bit
		CHECK_EQUAL(0xf45e775d03fde5b2, readWriteHashDpeg("fruit_24bit.tga"));// RGB RLE 24 bit
		CHECK_EQUAL(0xf45e775d03fde5b2, readWriteHashDpeg("fruit_24bit_uc.tga")); // RGB 24 bit
		CHECK_EQUAL(0x299fa2b205e26946, readWriteHashDpeg("redsquare_16bit_uc.tga"));
		CHECK_EQUAL(0xc1294de2676dd194, readWriteHashDpeg("xing_b16.tga"));
		CHECK_EQUAL(0x5bd0354cd775afac, readWriteHashDpeg("xing_b24.tga")); 
//		CHECK_EQUAL(0, readWriteHashDpeg("xing_b32.tga")); // Looks odd, but photoshop does the same and gimp just shows a transparent image
		CHECK_EQUAL(0xc1294de2676dd194, readWriteHashDpeg("xing_t16.tga")); 
		CHECK_EQUAL(0x5bd0354cd775afac, readWriteHashDpeg("xing_t24.tga")); 
//		CHECK_EQUAL(0, readWriteHashDpeg("xing_t32.tga")); // Looks odd, but photoshop does the same and gimp just shows a transparent image
		END_TEST();
	}

	TEST(tgaColormap)
	{
		START_TEST();

		CHECK_EQUAL(0x5909e09b6aa76af6, readWriteHashDpeg("fruit_8bit_uc.tga")); // COLORMAP
		CHECK_EQUAL(0x5909e09b6aa76af6, readWriteHashDpeg("fruit_8bit.tga")); // COLORMAP RLE
		CHECK_EQUAL(0x481e4de73e233786, readWriteHashDpeg("UCM8.TGA"));	// COLORMAP
		CHECK_EQUAL(0x481e4de73e233786, readWriteHashDpeg("CCM8.TGA")); // COLORMAP  RLE

		END_TEST();
	}

	TEST(tgaGray)
	{
		START_TEST();
		CHECK_EQUAL(0xc30860109b2f1b10, readWriteHashDpeg("UBW8.TGA"));	// GRAY
		CHECK_EQUAL(0xc30860109b2f1b10, readWriteHashDpeg("CBW8.TGA")); // GRAY  RLE
		END_TEST();
	}

}

#endif