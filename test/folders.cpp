#include "folders.h"
#ifdef _WIN32
// VS 2013 supports experimental/filesystem!
#  include <experimental/filesystem>
   namespace fs = std::experimental::filesystem
#else
// gcc 4.9 doesn't have filesystem just yet, so use boost here.
//#  include "boost/filesystem"
//   namespace fs = boost::filesystem
#endif

void cleanOut()
{
//	fs::remove_all( TESTOUT );
//	fs::create_directories( TESTOUT );
}

void createFolder(const char* folder)
{
//	fs::create_directories( folder );
}

double total_test_time = 0;