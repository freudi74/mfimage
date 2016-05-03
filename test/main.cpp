#include <UnitTest++/UnitTest++.h>
#include <UnitTest++/TestReporterStdout.h>
#include <UnitTest++/XmlTestReporter.h>
#include <fstream>
//#include <unittest++/TestRepo.h>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// To add a test, simply put the following code in the a .cpp file of your choice:
//
// =================================
// Simple Test
// =================================
//
//  TEST(YourTestName)
//  {
//  }
//
// The TEST macro contains enough machinery to turn this slightly odd-looking syntax into legal C++, and automatically register the test in a global list. 
// This test list forms the basis of what is executed by RunAllTests().
//
// If you want to re-use a set of test data for more than one test, or provide setup/teardown for tests, 
// you can use the TEST_FIXTURE macro instead. The macro requires that you pass it a class name that it will instantiate, so any setup and teardown code should be in its constructor and destructor.
//
//  struct SomeFixture
//  {
//    SomeFixture() { /* some setup */ }
//    ~SomeFixture() { /* some teardown */ }
//
//    int testData;
//  };
// 
//  TEST_FIXTURE(SomeFixture, YourTestName)
//  {
//    int temp = testData;
//  }
//
// =================================
// Test Suites
// =================================
// 
// Tests can be grouped into suites, using the SUITE macro. A suite serves as a namespace for test names, so that the same test name can be used in two difference contexts.
//
//  SUITE(YourSuiteName)
//  {
//    TEST(YourTestName)
//    {
//    }
//
//    TEST(YourOtherTestName)
//    {
//    }
//  }
//
// This will place the tests into a C++ namespace called YourSuiteName, and make the suite name available to UnitTest++. 
// RunAllTests() can be called for a specific suite name, so you can use this to build named groups of tests to be run together.
// Note how members of the fixture are used as if they are a part of the test, since the macro-generated test class derives from the provided fixture class.
//
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


#include "folders.h"

// run all tests
int main(int argc, char **argv)
{
	cleanOut();
	createFolder( TESTOUT );
	createFolder( TESTOUT SEPARATOR "bmp" );
	createFolder( TESTOUT SEPARATOR "gif" );
	createFolder( TESTOUT SEPARATOR "jpg" );
	createFolder( TESTOUT SEPARATOR "jp2" );
	createFolder( TESTOUT SEPARATOR "tiff" );
	createFolder( TESTOUT SEPARATOR "libtiffpic" );
	createFolder( TESTOUT SEPARATOR "pcx" );
	createFolder( TESTOUT SEPARATOR "png_test_suite" );

#if 0
	std::ofstream xml(TESTOUT SEPARATOR "reports.xml");
	UnitTest::XmlTestReporter reporter(xml);
	UnitTest::TestRunner runner(reporter);
	auto pred = []( UnitTest::Test* t) { 
        return std::string(t->m_details.testName) == "MyTest"; 
    };
	auto predTrue = []( UnitTest::Test* t) { 
        return true; 
    };
    return runner.RunTestsIf( UnitTest::Test::GetTestList(), 0, predTrue, 0 );
#else
	return UnitTest::RunAllTests();
#endif
}
