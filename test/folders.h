#pragma once

#include <iostream>
#include <chrono>

#define TESTDATA "/home/marco/dev/build/mfimage/testdata"
#define TESTOUT  "/home/marco/dev/build/mfimage/test/out"
#define SEPARATOR "/"

void cleanOut();
void createFolder( const char* folder );

using namespace std::chrono;

#define START_TEST() std::cout << std::endl; \
std::cout << "######################" << std::endl; \
std::cout << "# TEST: " << m_details.testName << std::endl; \
time_point<high_resolution_clock> testStart = high_resolution_clock::now();

#define END_TEST() duration<double> testdur = high_resolution_clock::now()- testStart; \
std::cout << "# DONE: " << m_details.testName << " - duration: " << std::fixed << testdur.count() << " sec" << std::endl;

