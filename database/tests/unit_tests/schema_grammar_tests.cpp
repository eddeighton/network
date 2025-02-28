//  Copyright (c) Deighton Systems Limited. 2022. All Rights Reserved.
//  Author: Edward Deighton
//  License: Please see license.txt in the project root folder.

//  Use and copying of this software and preparation of derivative works
//  based upon this software are permitted. Any copy of this software or
//  of any derivative work must include the above copyright notice, this
//  paragraph and the one after it.  Any distribution of this software or
//  derivative works must comply with all applicable laws.

//  This software is made available AS IS, and COPYRIGHT OWNERS DISCLAIMS
//  ALL WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE
//  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
//  PURPOSE, AND NOTWITHSTANDING ANY OTHER PROVISION CONTAINED HEREIN, ANY
//  LIABILITY FOR DAMAGES RESULTING FROM THE SOFTWARE OR ITS USE IS
//  EXPRESSLY DISCLAIMED, WHETHER ARISING IN CONTRACT, TORT (INCLUDING
//  NEGLIGENCE) OR STRICT LIABILITY, EVEN IF COPYRIGHT OWNERS ARE ADVISED
//  OF THE POSSIBILITY OF SUCH DAMAGES.

#include "database/compiler/grammar.hpp"

#include <gtest/gtest-param-test.h>
#include <gtest/gtest.h>

#include <utility>
#include <sstream>

// IdentifierAccept
class IdentifierAccept : public ::testing::TestWithParam< std::string >
{
protected:
    std::string str;
};

TEST_P( IdentifierAccept, Basic )
{
    std::string            s      = GetParam();
    db::schema::Identifier result = db::schema::parse< db::schema::Identifier >( s );
    ASSERT_EQ( result, s );
}

INSTANTIATE_TEST_SUITE_P(
    Identifier_Accept,
    IdentifierAccept,
    ::testing::Values( "a", "B", "a1", "m_1", "TheQuickBrownFoxJumpedOverTheLazyDoc1234567890" ) );

// IdentifierReject
class IdentifierReject : public ::testing::TestWithParam< std::string >
{
protected:
    std::string str;
};

TEST_P( IdentifierReject, Basic )
{
    db::schema::Identifier  id;
    std::ostringstream      osError;
    db::schema::ParseResult result = db::schema::parse( GetParam(), id, osError );
    ASSERT_FALSE( result.bSuccess );
}

INSTANTIATE_TEST_SUITE_P(
    Identifier_Reject,
    IdentifierReject,
    ::testing::Values( "1", "_", "&", "!", "£", "$", "%", "^", "&", ",", "(", ")", "123_", "=123=" ) );

// IdentifierListAccept
class IdentifierListAccept : public ::testing::TestWithParam< std::string >
{
protected:
    std::string str;
};

TEST_P( IdentifierListAccept, Basic )
{
    std::string                s      = GetParam();
    db::schema::IdentifierList result = db::schema::parse< db::schema::IdentifierList >( s );
    std::ostringstream         os;
    os << result;
    ASSERT_EQ( os.str(), s );
}

INSTANTIATE_TEST_SUITE_P( IdentifierList_Accept,
                          IdentifierListAccept,
                          ::testing::Values( "a", "a::b", "abc::def::efg" ) );

// TypeAccept
class TypeAccept : public ::testing::TestWithParam< std::string >
{
protected:
    std::string str;
};

TEST_P( TypeAccept, Basic )
{
    db::schema::Type        type;
    std::ostringstream      osError;
    db::schema::ParseResult result = db::schema::parse( GetParam(), type, osError );
    std::cout << type << std::endl;
    ASSERT_TRUE( result.bSuccess );
}

INSTANTIATE_TEST_SUITE_P( Type_Accept,
                          TypeAccept,
                          ::testing::Values( "a", "a< b::c >", "a::b< b, c, d >", "a< b< e >, c< f, g >, d >" ) );

// TypeReject
class TypeReject : public ::testing::TestWithParam< std::string >
{
protected:
    std::string str;
};

TEST_P( TypeReject, Basic )
{
    db::schema::Type        type;
    std::ostringstream      osError;
    db::schema::ParseResult result = db::schema::parse( GetParam(), type, osError );
    ASSERT_FALSE( result.bSuccess );
}

INSTANTIATE_TEST_SUITE_P( Type_Reject, TypeReject, ::testing::Values( "a<", "a<<>", "<a>", "a<>", ">a<", "a<<b>>" ) );

// StageAccept
class StageAccept : public ::testing::TestWithParam< std::string >
{
protected:
    std::string str;
};

TEST_P( StageAccept, Basic )
{
    db::schema::Stage       stage;
    std::ostringstream      osError;
    db::schema::ParseResult result = db::schema::parse( GetParam(), stage, osError );
    ASSERT_TRUE( result.bSuccess );
}

INSTANTIATE_TEST_SUITE_P( Stage_Accept,
                          StageAccept,
                          ::testing::Values( "stage test{}",
                                             "stage test{ file someFile; global accessor "
                                             "ref< foobar >; }" ) );

// PropertyAccept
class PropertyAccept : public ::testing::TestWithParam< std::string >
{
protected:
    std::string str;
};

TEST_P( PropertyAccept, Basic )
{
    db::schema::Property    property;
    std::ostringstream      osError;
    db::schema::ParseResult result = db::schema::parse( GetParam(), property, osError );
    std::cout << property << std::endl;
    ASSERT_TRUE( result.bSuccess );
}

INSTANTIATE_TEST_SUITE_P( Property_Accept,
                          PropertyAccept,
                          ::testing::Values( "test map< ref< a::b >, value< std::string >, pred< "
                                             "compare > > -> somefile" ) );

// ObjectAccept
class ObjectAccept : public ::testing::TestWithParam< std::string >
{
protected:
    std::string str;
};

TEST_P( ObjectAccept, Basic )
{
    db::schema::Object      object;
    std::ostringstream      osError;
    db::schema::ParseResult result = db::schema::parse( GetParam(), object, osError );
    std::cout << object << std::endl;
    ASSERT_TRUE( result.bSuccess );
}

// clang-format off
INSTANTIATE_TEST_SUITE_P( Property_Accept,
ObjectAccept,

::testing::Values(

"object test -> file{}",

"object test -> file"
"{"
"   value name;"
"}",

"object test -> somefile"
"{"
"    ref< a > context -> otherfile;"
"    value< std::string > name;"
"    map< ref< a >, value< bool >, pref< compare > > mapping;"
"}"

) );
// clang-format on

// NamespaceAccept
class NamespaceAccept : public ::testing::TestWithParam< std::string >
{
protected:
    std::string str;
};

TEST_P( NamespaceAccept, Basic )
{
    db::schema::Namespace   namespace_;
    std::ostringstream      osError;
    db::schema::ParseResult result = db::schema::parse( GetParam(), namespace_, osError );
    std::cout << namespace_ << std::endl;
    ASSERT_TRUE( result.bSuccess );
}

// clang-format off
INSTANTIATE_TEST_SUITE_P(
    Namespace_Accept,
    NamespaceAccept,
::testing::Values( 
    
"namespace test{}", 

"namespace test"
"{"
"    namespace test{}"
"    namespace test{}"
"}",

"namespace test"
"{"
"   object test -> somefile"
"   {"
"       value< std::string > name;"
"   }"
"   namespace test{}"
"}"

) );
// clang-format on

// SchemaAccept
class SchemaAccept : public ::testing::TestWithParam< std::string >
{
protected:
    std::string str;
};

TEST_P( SchemaAccept, Basic )
{
    db::schema::Schema      schema;
    std::ostringstream      osError;
    db::schema::ParseResult result = db::schema::parse( GetParam(), schema, osError );
    std::cout << schema << std::endl;
    ASSERT_TRUE( result.bSuccess );
}

// clang-format off
INSTANTIATE_TEST_SUITE_P(
    Schema_Accept,
    SchemaAccept,
::testing::Values( 
    
"namespace test{}", 

"stage test"
"{"
"  file fileone;"
"  dependency d1;"
"  source foo;"
"  global accessor ref< Parser::Root >;"
"  persource accessor ref< Parser::Root >;"
"}",

"stage test1"
"{"
"  file fileone;"
"}"
"namespace test"
"{"
"   object test : foobar -> A::B"
"   {"
"       value< std::string > name1;"
"       value< std::string > name2 -> A::C;"
"       value< std::string > name3;"
"   }"
"}"
"stage test2"
"{"
"  file fileone;"
"}"
"namespace test2"
"{"
"   object test -> A::B"
"   {"
"       value< std::string > name;"
"   }"
"}"

) );
// clang-format on
