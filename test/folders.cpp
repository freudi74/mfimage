#include "folders.h"
#include "../src/Image.h"
#include <iomanip>

#ifdef _WIN32
// VS 2013 supports experimental/filesystem!
#  include <filesystem>
namespace fs = std::tr2::sys;
#else
// gcc 4.9 doesn't have filesystem just yet, so use boost here.
//#  include "boost/filesystem"
//   namespace fs = boost::filesystem;
#endif

void cleanOut()
{
//	fs::path testout(TESTOUT);
/*	fs::remove_all( testout );
	fs::create_directories( testout );
	fs::create_directories(testout / fs::path("bmp") );
	fs::create_directories(testout / fs::path("gif"));
	fs::create_directories(testout / fs::path("jp2"));
	fs::create_directories(testout / fs::path("jpg"));
	fs::create_directories(testout / fs::path("libtiff"));
	fs::create_directories(testout / fs::path("pcx"));
	fs::create_directories(testout / fs::path("png_test_suite"));
	fs::create_directories(testout / fs::path("tiff"));
	*/
	//	fs::create_directories(testout / "gif");
//	fs::create_directories(testout / "jp2");
//	fs::create_directories(testout / "jpg");
//	fs::create_directories(testout / "libtiff");
//	fs::create_directories(testout / "pcx");
//	fs::create_directories(testout / "png_test_suite");
//	fs::create_directories(testout / "tiff");
}

void createFolder(const char* folder)
{
//	fs::create_directories( folder );
}

double total_test_time = 0;

size_t readWriteHash(const std::string & fileNameOnly, const std::string & inPath, const std::string & outPath, bool writeOutFile, bool printResults/*=true*/ )
{
	Image img;
	ImageCoderProperties props;
	props.compression = TIFF_Compression::NONE;
	props.embedTimestamp = false;
	props.embedOtherInfo = false;
	img.read(inPath, 1);
	if ( writeOutFile )
		img.write( outPath, IE_TIFF, &props);
	size_t hash = img.calcHash();
	size_t namePad = static_cast<size_t>(std::max(0, 20 - (int)fileNameOnly.length()));
	if ( printResults )
		std::cout << fileNameOnly << std::string(namePad, ' ') << "\t0x" << std::hex << std::setw(sizeof(size_t)* 2) << std::setfill('0') << hash << std::endl;
	return hash;

}
