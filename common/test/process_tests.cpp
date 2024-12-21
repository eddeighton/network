
#include "common/process.hpp"

#include "boost/filesystem.hpp"

#include <gtest/gtest.h>

#ifdef _WIN32
static const std::string exeName = "common_tests.exe";
static const std::string expected =
    R"EXPECTED(Allowed options:
  --help                produce help message
  --filter arg          filter string to select subset of tests
  --gtest_filter arg    filter string to select subset of tests
  --xsl arg             xsl file to generate html report
  --repeats arg         number of times to repeat tests
  --report [=arg(=1)]   generate xml report
  --wait [=arg(=1)]     wait once complete to keep terminal open
  --debug [=arg(=1)]    enable debugging
  --break [=arg(=1)]    break at startup to enable attaching debugger
  --cout [=arg(=1)]     display standard output

)EXPECTED";
#else
static const std::string exeName = "common_tests";
static const std::string expected =
    R"EXPECTED(Allowed options:
  --help                produce help message
  --filter arg          filter string to select subset of tests
  --gtest_filter arg    filter string to select subset of tests
  --xsl arg             xsl file to generate html report
  --repeats arg         number of times to repeat tests
  --report [=arg(=1)]   generate xml report
  --wait [=arg(=1)]     wait once complete to keep terminal open
  --debug [=arg(=1)]    enable debugging
  --break [=arg(=1)]    break at startup to enable attaching debugger
  --cout [=arg(=1)]     display standard output

)EXPECTED";
#endif

/*
TEST( ProcessTests, BasicOutput )
{
    ASSERT_TRUE( boost::filesystem::exists( exeName ) )
        << "Unit test could not find executable " << exeName << " - are you running from correct cwd?";

    std::ostringstream osCmd;
    osCmd << exeName << " --help";

    std::string strOutput, strError;

#ifdef _WIN32
    const int iExitCode = common::runProcess( osCmd.str(), strOutput, strError );
    ASSERT_EQ( iExitCode, EXIT_SUCCESS );
    // remove \r
    strOutput.erase( std::remove( strOutput.begin(), strOutput.end(), '\r' ), strOutput.end() );
    strError.erase( std::remove( strError.begin(), strError.end(), '\r' ), strError.end() );
#else
    const int iExitCode = common::runProcess( osCmd.str(), strOutput, strError );
    ASSERT_EQ( iExitCode, EXIT_SUCCESS );
#endif
    ASSERT_EQ( expected, strOutput );
    ASSERT_EQ( "", strError );
}*/
/*
TEST( ProcessTests, BasicError )
{
    ASSERT_TRUE( boost::filesystem::exists( exeName ) )
        << "Unit test could not find executable " << exeName << " - are you running from correct cwd?";

    std::ostringstream osCmd;
    osCmd << exeName << " --foobar";

    std::string strOutput, strError;

#ifdef _WIN32
    const int iExitCode = common::runProcess( osCmd.str(), strOutput, strError );
    ASSERT_EQ( iExitCode, EXIT_FAILURE );
    // remove \r
    strOutput.erase( std::remove( strOutput.begin(), strOutput.end(), '\r' ), strOutput.end() );
    strError.erase( std::remove( strError.begin(), strError.end(), '\r' ), strError.end() );
#else
    const int iExitCode = common::runProcess( osCmd.str(), strOutput, strError );
    ASSERT_EQ( iExitCode, EXIT_FAILURE );
#endif
    ASSERT_EQ( "", strOutput );
    ASSERT_EQ( "Exception calling main: unrecognised option '--foobar'\n", strError );
}*/

