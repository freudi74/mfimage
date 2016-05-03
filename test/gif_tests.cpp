#include <UnitTest++/UnitTest++.h>

#if !defined(_WIN32)
#  include <unistd.h>
#endif
#include <stdio.h>
#include <iomanip>
#include <iostream>

#include "../src/Image.h"
#include <stdexcept>

#include "folders.h"

#ifndef SKIP_GIF_SUITE

#define TEST_FILES TESTDATA SEPARATOR "gif" SEPARATOR
#define TEST_OUT   TESTOUT  SEPARATOR "gif" SEPARATOR
TEST(Gif87aRead)
{
	START_TEST();
	Image img;
	img.read( TEST_FILES "bugs.gif" );
	CHECK_EQUAL( 0, img.calcHash() );
	img.write( TEST_OUT "bugs.tiff", IE_TIFF );
/*	ImageCoderProperties props;
	props.compression = TIFF_Compression::LZW;
	img.write( TEST_OUT "bugs_c_lzw.tiff", IE_TIFF, &props );
	props.compression = TIFF_Compression::DEFLATE;
	img.write( TEST_OUT "bugs_c_deflate.tiff", IE_TIFF, &props );
	props.compression = TIFF_Compression::LZMA;
	img.write( TEST_OUT "bugs_c_lzma.tiff", IE_TIFF, &props );
	props.compression = TIFF_Compression::PACKBITS;
	img.write( TEST_OUT "bugs_c_pb.tiff", IE_TIFF, &props );
*/	// compare results against a reference?
	END_TEST();

}

TEST(Gif89aRead)
{
	START_TEST();

	Image imgTransparent;
	imgTransparent.read( TEST_FILES "transbugs.gif" );
	imgTransparent.write( TEST_OUT "transbugs.tif", IE_TIFF );
	Image imgMultipageFirst;
	imgMultipageFirst.read( TEST_FILES "trnscool.gif" );
	for ( int i=1; i<=13; i++ )
	{
		Image imgMultipageAll;
		imgMultipageAll.read( TEST_FILES "trnscool.gif", i );
		std::stringstream outfile;
		outfile << TEST_OUT << "trnscool-" << std::dec << std::setfill('0') << std::setw(2) << i << ".tif";
		imgMultipageAll.write( outfile.str(), IE_TIFF );
	}
	// compare results against a reference?
	END_TEST();

}

#endif