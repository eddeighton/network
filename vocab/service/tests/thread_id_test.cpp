
#include "vocab/service/thread_id.hpp"

#include <gtest/gtest.h>

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/interprocess/streams/vectorstream.hpp>
#include <limits>

TEST( ThreadID, Basic )
{
    using namespace mega::service;

    const ThreadID t1{};
    ThreadID v{};

    ASSERT_EQ( t1.getValue(), 0 );
    ASSERT_EQ( t1.getValue(), v.getValue() );

    v++;

    const ThreadID t2{ v };

    ASSERT_EQ( t2.getValue(), 1 );
    ASSERT_EQ( t2.getValue(), v.getValue() );

    std::ostringstream os;
    os << t2;
    ASSERT_EQ( os.str(), "0x01_T" );

    ASSERT_EQ( t2, 0x01_T );
}

