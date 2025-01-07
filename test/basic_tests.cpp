
#include "test/test_object.hpp"
#include "test/test_factory.hpp"

#include "service/logical_thread.hpp"
#include "service/network.hpp"
#include "service/registry.hpp"
#include "service/ptr.hpp"

#include <gtest/gtest.h>

#include <utility>
#include <iostream>

TEST( Service, Basic )
{
    using namespace mega::service;
    using namespace mega::test;

    Access access;
    
    access.writeRegistry()->setCreationCallback([](Registration){});

    MP mp{};

    Network network;

    LogicalThread::registerFiber(mp);
    LogicalThread& thread = LogicalThread::get();

    OTestFactory testFactory( access );

    Ptr< TestFactory > pFactory = testFactory.getPtr();
    LOG( "Created TestFactory: " << testFactory.getMPTFO() );
    Ptr< mega::test::Test > pTest2 = pFactory->create_test();
    LOG( "Test returned: " << pTest2->test1() );

    boost::fibers::fiber test( [&]()
    {
        LogicalThread::registerFiber(mp);
        try
        {
            Ptr< mega::test::Test > pTest = pFactory->create_test();
            LOG( "Test returned: " << pTest->test1() );
        }
        catch(std::exception& ex)
        {
            LOG( "Caught exception: " << ex.what() <<
                " In: " << LogicalThread::get().getMPTF() );
        }
        thread.stop();
    });

    LogicalThread::get().runMessageLoop();
    test.join();
}

