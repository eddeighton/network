
//  Copyright (c) Deighton Systems Limited. 2022. All Rights Reserved.
//  Author: Edward Deighton
//  License: Please see license.txt in the project root folder.

//  Use and copying of this software and preparation of derivative
//  works based upon this software are permitted. Any copy of this
//  software or of any derivative work must include the above
//  copyright notice, this paragraph and the one after it.  Any
//  distribution of this software or derivative works must comply with
//  all applicable laws.

//  This software is made available AS IS, and COPYRIGHT OWNERS
//  DISCLAIMS ALL WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT
//  LIMITATION THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
//  FOR A PARTICULAR PURPOSE, AND NOTWITHSTANDING ANY OTHER PROVISION
//  CONTAINED HEREIN, ANY LIABILITY FOR DAMAGES RESULTING FROM THE
//  SOFTWARE OR ITS USE IS EXPRESSLY DISCLAIMED, WHETHER ARISING IN
//  CONTRACT, TORT (INCLUDING NEGLIGENCE) OR STRICT LIABILITY, EVEN IF
//  COPYRIGHT OWNERS ARE ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.

#include "controller/locator.hpp"

#include <gtest/gtest.h>

#define CHECK_FAILED( value )                                 \
    std::ostringstream      osError;                          \
    const auto              p = GetParam();                   \
    Controller::ParseResult result                            \
        = Controller::parse( GetParam(), value, osError );    \
    ASSERT_FALSE( result.bSuccess&& result.iterReached.base() \
                  == p.end() );

namespace
{
// NameAccept
class NameAccept : public ::testing::TestWithParam< std::string >
{
protected:
    std::string str;
};

TEST_P( NameAccept, Basic )
{
    std::string      s = GetParam();
    Controller::Name result
        = Controller::parse< Controller::Name >( s );
    ASSERT_EQ( result, s );
}

INSTANTIATE_TEST_SUITE_P( Name_Accept,
                          NameAccept,
                          ::testing::Values( "a",
                                             "B",
                                             "a1",
                                             "m_1",
                                             "TheQuickBrownFoxJumpedO"
                                             "verTheLazyDoc123456789"
                                             "0" ) );
} // namespace

namespace
{
// NameReject
class NameReject : public ::testing::TestWithParam< std::string >
{
protected:
    std::string str;
};

TEST_P( NameReject, Basic )
{
    Controller::Name id;
    CHECK_FAILED( id );
}

INSTANTIATE_TEST_SUITE_P( Name_Reject,
                          NameReject,
                          ::testing::Values( " ",
                                             "&",
                                             " a",
                                             "a ",
                                             "a a",
                                             "!",
                                             "£",
                                             "$",
                                             "%",
                                             "^",
                                             "&",
                                             ",",
                                             "(",
                                             ")",
                                             "=123=" ) );

} // namespace

namespace
{
// ExtensionAccept
class ExtensionAccept : public ::testing::TestWithParam< std::string >
{
protected:
    std::string str;
};

TEST_P( ExtensionAccept, Basic )
{
    std::string           s = GetParam();
    Controller::Extension result
        = Controller::parse< Controller::Extension >( s );
    ASSERT_EQ( result, s );
}

INSTANTIATE_TEST_SUITE_P( Extension_Accept,
                          ExtensionAccept,
                          ::testing::Values( ".B",
                                             ".a1",
                                             ".m_1",
                                             ".TheQuickBrownFoxJumped"
                                             "OverTheLazyDoc123456789"
                                             "0" ) );
} // namespace

namespace
{
// ExtensionReject
class ExtensionReject : public ::testing::TestWithParam< std::string >
{
protected:
    std::string str;
};

TEST_P( ExtensionReject, Basic )
{
    Controller::Extension id;
    CHECK_FAILED( id );
}

INSTANTIATE_TEST_SUITE_P( Extension_Reject,
                          ExtensionReject,
                          ::testing::Values( ".",
                                             ". a",
                                             ".a ",
                                             "a",
                                             "abc",
                                             "ABC",
                                             "1",
                                             "_",
                                             "&",
                                             "!",
                                             "£",
                                             "$",
                                             "%",
                                             "^",
                                             "&",
                                             ",",
                                             "(",
                                             ")",
                                             "123_",
                                             "=123=" ) );

} // namespace

namespace
{
// PathAccept
class PathAccept : public ::testing::TestWithParam< std::string >
{
protected:
    std::string str;
};

TEST_P( PathAccept, Basic )
{
    std::string      s = GetParam();
    Controller::Path result
        = Controller::parse< Controller::Path >( s );
    ASSERT_EQ( result.str(), s );
}

INSTANTIATE_TEST_SUITE_P( Path_Accept,
                          PathAccept,
                          ::testing::Values( "a.txt",
                                             "a.a",
                                             "a.a.a",
                                             "_.1",
                                             "TheQuickBrownFoxJumpedO"
                                             "verTheLazyDog.txt.gz."
                                             "zip",
                                             "some_source_code.ipp."
                                             "cpp.cxx.jinja" ) );
} // namespace

namespace
{
// PathReject
class PathReject : public ::testing::TestWithParam< std::string >
{
protected:
    std::string str;
};

TEST_P( PathReject, Basic )
{
    Controller::Path id;
    CHECK_FAILED( id );
}

INSTANTIATE_TEST_SUITE_P(
    Path_Reject,
    PathReject,
    ::testing::Values( ".txt", "@.txt", "..", "abs .txt" ) );

} // namespace

namespace
{

using namespace Controller;

struct TestData
{
    std::string         str;
    std::vector< Path > expected;
};

// LineAccept
class LineAccept : public ::testing::TestWithParam< TestData >
{
protected:
    TestData data;
};

TEST_P( LineAccept, Basic )
{
    const TestData&  d = GetParam();
    Controller::Line result
        = Controller::parse< Controller::Line >( d.str );
    ASSERT_EQ( result.m_paths, d.expected );
}

using namespace std::string_literals;

INSTANTIATE_TEST_SUITE_P(
    Line_Accept,
    LineAccept,
    ::testing::Values(
        TestData{
            "a.txt"s,
            { Path{ Name{ "a"s }, {}, { Extension{ ".txt"s } } } } },
        TestData{
            "a.txt b.txt c.txt",
            {
                Path{ Name{ "a"s }, {}, { Extension{ ".txt"s } } },
                Path{ Name{ "b"s }, {}, { Extension{ ".txt"s } } },
                Path{ Name{ "c"s }, {}, { Extension{ ".txt"s } } },
            } }

        //                                            TestData{
        //                                            "./a/b/c/s.cpp",
        //                                                       {
        //                                                           Path{ Dot{},
        //                                                           {
        //                                                              Name{ "a"s },
        //                                                              Name{ "b"s },
        //                                                              Name{ "c"s },
        //                                                              Name{ "s"s }
        //                                                           },
        //                                                           { Extension{ ".cpp"s } } }
        //
        //                                                           } }
        //
        ) );
} // namespace
