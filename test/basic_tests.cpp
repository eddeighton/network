
#include "test/test_object.hpp"
#include "test/test_factory.hpp"

#include "test/test_object.hpp"
#include "test/service/testfactory.proxy.hxx"
#include "test/service/test.proxy.hxx"

#include "service/rtti.hpp"
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
    
    MP mp{};

    Network network( mp );

    LogicalThread::registerFiber(network.getMP());
    LogicalThread& thread = LogicalThread::get();

    OTestFactory testFactory(network);

    Ptr< TestFactory > pFactory = testFactory.getPtr();
    std::cout << "Created TestFactory: " << testFactory.getMPO() << std::endl;
    Ptr< mega::test::Test > pTest2 = pFactory->create_test();
    std::cout << "Test returned: " << pTest2->test1() << std::endl;

    boost::fibers::fiber test( [&]()
    {
        LogicalThread::registerFiber(network.getMP());
        try
        {
            Ptr< mega::test::Test > pTest = pFactory->create_test();
            std::cout << "Test returned: " << pTest->test1() << std::endl;
        }
        catch( std::exception& ex )
        {
            std::cout << "Caught exception: " << ex.what() <<
                " In: " << LogicalThread::get().getMPTF() << std::endl;
        }
        thread.stop();
    });

    LogicalThread::get().runMessageLoop();
    test.join();
}

