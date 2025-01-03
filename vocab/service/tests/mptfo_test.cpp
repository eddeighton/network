

#include "vocab/service/mptfo.hpp"

#include <gtest/gtest.h>


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

