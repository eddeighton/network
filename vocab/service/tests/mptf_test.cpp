
#include "vocab/service/mptf.hpp"

#include <gtest/gtest.h>

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/interprocess/streams/vectorstream.hpp>
#include <limits>

TEST( MPTF, Basic )
{
    using namespace mega::service;

    MPTF m1{ {}, {}, {}, FiberID{ 0 } };
    MPTF m2{ {}, {}, {}, FiberID{ 1 } };
    ASSERT_TRUE( m1 < m2 );
}

TEST( MPTF, Basic_Order )
{
    using namespace mega::service;

    MPTF m1{ {}, {}, {}, FiberID{ 2 } };
    MPTF m2{ {}, {}, {}, FiberID{ 1 } };
    ASSERT_TRUE( m2 < m1 );
}

TEST( MPTF, Serialise )
{
    using namespace mega::service;

    const std::vector< MPTF > expected = {
        // clang-format off
        MPTF{},
        MPTF{ std::numeric_limits<MPTF::ValueType>::min() },
        MPTF{ std::numeric_limits<MPTF::ValueType>::max() },
        MPTF{ MachineID{1}, ProcessID{2}, ThreadID{3}, FiberID{4} },
        MPTF{ MachineID{4}, ProcessID{3}, ThreadID{2}, FiberID{1} }
        // clang-format on
    };

    std::vector< MPTF > result;

    {
        boost::interprocess::basic_vectorbuf< std::vector< char > >
                                        buffer;
        boost::archive::binary_oarchive saveArchive( buffer );

        saveArchive & expected;

        boost::archive::binary_iarchive loadArchive( buffer );

        loadArchive & result;
    }

    ASSERT_EQ( expected, result );
}

TEST( MPTF, IoStreams )
{
    using namespace std::string_literals;
    using namespace mega::service;

    const MPTF m{ MachineID{1}, ProcessID{2}, ThreadID{3}, FiberID{4} };
    std::ostringstream os;
    os << m;

    ASSERT_EQ( os.str(), "0x00000001_M.0x02_P.0x03_T.0x04_F"s );
}

