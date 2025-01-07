
#include "test/test_object.hpp"
#include "test/test_factory.hpp"
#include "service/connectivity.hpp"

#include "service/daemon.hpp"
#include "service/connect.hpp"
#include "service/logical_thread.hpp"
#include "service/network.hpp"
#include "service/registry.hpp"
#include "service/ptr.hpp"

#include "common/log.hpp"

#include <boost/process.hpp>
#include <boost/asio/io_service.hpp>

#include <gtest/gtest.h>

#include <utility>
#include <iostream>
#include <string>
#include <chrono>

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

    std::promise< void > waitForDaemonStart;
    std::future< void > waitForDaemonStartFut = waitForDaemonStart.get_future();
    std::thread daemonThread(
        [&waitForDaemonStart]
        {
            LogicalThread::registerThread();
            Daemon daemon( {}, PortNumber{ 1234 } );
            OConnectivity connectivity(daemon);
            waitForDaemonStart.set_value();
            daemon.run();
        });

    {
        waitForDaemonStartFut.get();
        std::cout << "Client started" << std::endl;
        Connect con( {}, PortNumber{ 1234 } );

        std::cout << "Client started 2" << std::endl;
        auto pConnectivity = con.readRegistry()->one< Connectivity >( MP{} );
        pConnectivity->shutdown();
        std::cout << "Client complete" << std::endl;
    }

    daemonThread.join();
}

TEST( Service, CreateTest )
{
    using namespace mega::service;
    using namespace mega::test;

    LogicalThread::registerThread();

    std::promise< void > waitForDaemonStart;
    std::future< void > waitForDaemonStartFut = waitForDaemonStart.get_future();
    std::thread daemon(
        [&waitForDaemonStart]
        {
            LogicalThread::registerThread();
            Daemon daemon( {}, PortNumber{ 1234 } );
            OConnectivity connectivity(daemon);
            OTestFactory testFactory( daemon );
            waitForDaemonStart.set_value();
            daemon.run();
        });

    {
        waitForDaemonStartFut.get();
        Connect con( {}, PortNumber{ 1234 } );

        auto pTestFactory = con.readRegistry()->one< TestFactory >( MP{} );
        auto pTest = pTestFactory->create_test();
        ASSERT_TRUE(pTest);
        // for( int i = 0; i != 1000; i++ )
        {
            ASSERT_EQ( pTest->test1(), "Hello World"s );
            ASSERT_EQ( pTest->test2( 123 ), 123 );
            ASSERT_EQ( pTest->test3( "This"s ), "This"s );
            ASSERT_EQ( pTest->test3( ""s ), ""s );
        }

        auto pConnectivity = con.readRegistry()->one< Connectivity >( MP{} );
        pConnectivity->shutdown();
    }

    daemon.join();
}

TEST( Service, InterConnect )
{
    using namespace mega::service;
    using namespace mega::test;

    LogicalThread::registerThread();

    std::promise< void > waitForDaemonStart;
    std::future< void > waitForDaemonStartFut = waitForDaemonStart.get_future();
    std::thread daemon(
        [&waitForDaemonStart]
        {
            {
                LogicalThread::registerThread();
                Daemon daemon( {}, PortNumber{ 1234 } );
                OConnectivity connectivity(daemon);
                OTestFactory testFactory( daemon );
                waitForDaemonStart.set_value();
                daemon.run();
            }
            LOG( "daemon shutting down" );
        });
    waitForDaemonStartFut.get();

    std::promise< LogicalThread* > p;
    auto f = p.get_future();
    std::thread client1(
        [&]
        {
            {
                LogicalThread::registerThread();
                Connect con( {}, PortNumber{ 1234 } );
                p.set_value( &LogicalThread::get() );
                con.run();
            }
            LOG( "client1 shut down" );
        });

    {
        {
            Connect con( {}, PortNumber{ 1234 } );
            auto pConnectivity = con.readRegistry()->one< Connectivity >( MP{} );
            pConnectivity->shutdown();
        }
        LOG( "client2 shut down" );
    }

    f.get()->stop();
    client1.join();
    daemon.join();
}

