

#include "vocab/service/mptfo.hpp"

#include <gtest/gtest.h>

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/interprocess/streams/vectorstream.hpp>
#include <limits>

TEST( MPTFO, Basic )
{
    using namespace mega::service;

    MPTFO m1{ {}, {}, {}, {}, ObjectID{0} };
    MPTFO m2{ {}, {}, {}, {}, ObjectID{1} };
    ASSERT_TRUE( m1 < m2 );
}

TEST( MPTFO, Basic_Order )
{
    using namespace mega::service;

    MPTFO m1{ {}, {}, {}, FiberID{2}, ObjectID{0} };
    MPTFO m2{ {}, {}, {}, FiberID{1}, ObjectID{1} };
    ASSERT_TRUE( m2 < m1 ); 
}

TEST( MPTFO, Serialise )
{
    using namespace mega::service;

    const std::vector< MPTFO > expected = {
        // clang-format off
        MPTFO{},
        MPTFO{ std::numeric_limits<MPTFO::ValueType>::min() },
        MPTFO{ std::numeric_limits<MPTFO::ValueType>::max() },
        MPTFO{ MachineID{1}, ProcessID{2}, ThreadID{3}, FiberID{4}, ObjectID{5}},
        MPTFO{ MachineID{5}, ProcessID{4}, ThreadID{3}, FiberID{2}, ObjectID{1}}
        // clang-format on
    };

    std::vector< MPTFO > result;

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

TEST( MPTFO, IoStreams )
{
    using namespace std::string_literals;
    using namespace mega::service;

    const MPTFO m{ MachineID{1}, ProcessID{2}, ThreadID{3}, FiberID{4}, ObjectID{5} };
    std::ostringstream os;
    os << m;

    ASSERT_EQ( os.str(), "0x00000001_M.0x02_P.0x03_T.0x04_F.0x05_O"s );
}

