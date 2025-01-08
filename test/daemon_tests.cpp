
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
            try
            {
                LogicalThread::registerThread();
                Daemon daemon( {}, PortNumber{ 1234 } );
                OConnectivity connectivity(daemon);
                waitForDaemonStart.set_value();
                daemon.run();
            }
            catch( Shutdown& ) { }
        });
    waitForDaemonStartFut.get();

    try
    {
        LOG( "Client started" );
        Connect con( {}, PortNumber{ 1234 } );

        LOG( "Client started 2" );
        auto pConnectivity = con.readRegistry()->one< Connectivity >( MP{} );
        pConnectivity->shutdown();
        con.run();
    }
    catch( Shutdown& )
    {
        LOG( "Client shutdown exception" );
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
            try
            {
                LogicalThread::registerThread();
                Daemon daemon( {}, PortNumber{ 1234 } );
                OConnectivity connectivity(daemon);
                OTestFactory testFactory( daemon );
                waitForDaemonStart.set_value();
                daemon.run();
            }
            catch( Shutdown& ) { }
        });
    waitForDaemonStartFut.get();

    try
    {
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
        con.run();
    }
    catch( Shutdown& )
    {
        LOG( "client shutdown exception" );
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
            try
            {
                LogicalThread::registerThread();
                Daemon daemon( {}, PortNumber{ 1234 } );
                OConnectivity connectivity(daemon);
                OTestFactory testFactory( daemon );
                waitForDaemonStart.set_value();
                daemon.run();
            }
            catch( Shutdown& ) { }
            LOG( "daemon shut down" );
        });
    waitForDaemonStartFut.get();

    std::thread client1(
        [&]
        {
            try
            {
                LogicalThread::registerThread();
                Connect con( {}, PortNumber{ 1234 } );
                con.run();
            }
            catch( Shutdown& ) { }
            LOG( "client1 shut down" );
        });

    std::thread client2(
        [&]
        {
            try
            {
                LogicalThread::registerThread();
                Connect con( {}, PortNumber{ 1234 } );
                con.run();
            }
            catch( Shutdown& ) { }
            LOG( "client2 shut down" );
        });

    std::thread client3(
        [&]
        {
            try
            {
                LogicalThread::registerThread();
                Connect con( {}, PortNumber{ 1234 } );
                con.run();
            }
            catch( Shutdown& ) { }
            LOG( "client3 shut down" );
        });

    {
        try
        {
            Connect con( {}, PortNumber{ 1234 } );
            auto pConnectivity = con.readRegistry()->one< Connectivity >( MP{} );
            pConnectivity->shutdown();
        }
        catch( Shutdown& ) { }
        LOG( "client4 shut down" );
    }

    client1.join();
    client2.join();
    client3.join();
    daemon.join();
}

