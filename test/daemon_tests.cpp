
#include "test/test_object.hpp"
#include "test/test_factory.hpp"
#include "service/connectivity.hpp"

#include "service/daemon.hpp"
#include "service/connect.hpp"
#include "service/logical_thread.hpp"
#include "service/network.hpp"
#include "service/registry.hpp"
#include "service/ptr.hpp"

#include <boost/process.hpp>
#include <boost/asio/io_service.hpp>

#include <gtest/gtest.h>

#include <utility>
#include <iostream>
#include <string>

using namespace std::string_literals;

TEST( Service, DaemonStartStop )
{
    using namespace mega::service;
    using namespace mega::test;

    Daemon daemon( {}, PortNumber{ 4321} );
}

TEST( Service, DaemonConnect )
{
    using namespace mega::service;
    using namespace mega::test;

    LogicalThread::registerThread();

    std::thread daemon(
        []
        {
            LogicalThread::registerThread();
            Daemon daemon( {}, PortNumber{ 1234 } );
            OConnectivity connectivity(daemon);
            daemon.run();
        });

    {
        Connect con( {}, PortNumber{ 1234 } );

        auto pConnectivity = con.readRegistry()->one< Connectivity >( MP{} );
        pConnectivity->shutdown();
    }

    daemon.join();
}

TEST( Service, CreateTest )
{
    using namespace mega::service;
    using namespace mega::test;

    LogicalThread::registerThread();

    std::thread daemon(
        []
        {
            LogicalThread::registerThread();
            Daemon daemon( {}, PortNumber{ 1234 } );
            OConnectivity connectivity(daemon);
            OTestFactory testFactory( daemon );
            daemon.run();
        });

    {
        Connect con( {}, PortNumber{ 1234 } );

        auto pTestFactory = con.readRegistry()->one< TestFactory >( MP{} );
        auto pTest = pTestFactory->create_test();
        ASSERT_TRUE(pTest);
        ASSERT_EQ( pTest->test1(), "Hello World"s );
        ASSERT_EQ( pTest->test2( 123 ), 123 );
        ASSERT_EQ( pTest->test3( "This"s ), "This"s );

        auto pConnectivity = con.readRegistry()->one< Connectivity >( MP{} );
        pConnectivity->shutdown();
    }

    daemon.join();
}

