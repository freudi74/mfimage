#pragma once

#include <iostream>
#include <chrono>

#ifdef _WIN32
#define SEPARATOR "\\"
#else
#define SEPARATOR "/"
#endif

#define TESTDATA ".." SEPARATOR "testdata"
#define TESTOUT  ".." SEPARATOR "test-out"

void cleanOut();
void createFolder( const char* folder );

using namespace std::chrono;

extern double total_test_time;

#define START_TEST() std::cout << std::endl; \
std::cout << "######################" << std::endl; \
std::cout << "# TEST: " << m_details.testName << std::endl; \
std::cout << std::endl; \
time_point<high_resolution_clock> testStart = high_resolution_clock::now(); \

#define END_TEST() duration<double> testdur = high_resolution_clock::now()- testStart; \
total_test_time += testdur.count(); \
std::cout << "# DONE: " << m_details.testName << " - duration: " << std::fixed << testdur.count() << " sec (Sum: " << std::fixed << total_test_time << " sec)" << std::endl;


size_t readWriteHash(const std::string & fileNameOnly, const std::string & inPath, const std::string & outPath, bool writeOutFile, bool printResults=true);

//#define SKIP_BMP_SUITE
//#define SKIP_GIF_SUITE
#define SKIP_JPEG_SUITE
#define SKIP_JPEG2000_SUITE
#define SKIP_PCX_SUITE
#define SKIP_PNG_SUITE
//#define SKIP_TGA_SUITE
#define SKIP_TIFF_SUITE


