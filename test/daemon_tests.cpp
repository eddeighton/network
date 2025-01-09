
#include "test/test_object.hpp"
#include "test/test_factory.hpp"
#include "service/connectivity.hpp"

#include "service/daemon.hpp"
#include "service/connect.hpp"
#include "service/logical_thread.hpp"
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

#include <boost/stacktrace.hpp>
// #include <stacktrace>

#include <csignal>

using namespace std::string_literals;

void signalHandler( int nSignum )
{
    std::cout << "Signal handler called" << std::endl;

    std::cout << boost::stacktrace::stacktrace() << std::endl;

    std::abort();
}

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

    LogicalThread::reset();

    std::unique_ptr< std::thread > pDaemonThread;

    try
    {

        std::promise< void > waitForDaemonStart;
        std::future< void > waitForDaemonStartFut = waitForDaemonStart.get_future();
        pDaemonThread = std::make_unique< std::thread >(
            [&waitForDaemonStart]
            {
                try
                {
                    Daemon daemon( {}, PortNumber{ 1234 } );
                    OConnectivity connectivity(daemon);
                    waitForDaemonStart.set_value();
                    daemon.run();
                }
                catch( Shutdown& ) {}
            });
        waitForDaemonStartFut.get();

        {
            try
            {
                Connect con( {}, PortNumber{ 1234 } );

                auto pConnectivity = con.readRegistry()->one< Connectivity >( MP{} );
                pConnectivity->shutdown();
                con.run();
            }
            catch( ::mega::service::Shutdown& ) {}
            catch( std::exception& ex )
            {
                LOG( "Unexpected exception in  client: " << ex.what() );
                FAIL();
            }
            catch( ... )
            {
                LOG( "Unknown exception in  client " );
                FAIL();
            }
        }

    }
    catch( std::exception& ex )
    {
        std::cout << "Unexpected exception: " << ex.what() << std::endl;
        FAIL();
    }
    pDaemonThread->join();
}

TEST( Service, CreateTest )
{

    std::signal( SIGSEGV, signalHandler );

    using namespace mega::service;
    using namespace mega::test;

    LogicalThread::reset();

    std::promise< void > waitForDaemonStart;
    std::future< void > waitForDaemonStartFut = waitForDaemonStart.get_future();
    std::thread daemon(
        [&waitForDaemonStart]
        {
            try
            {
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

        LOG( "Connect constructed starting test function calls" );

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
    catch( std::exception& ex )
    {
        LOG( "Unexpected exception: " << ex.what() );
        FAIL() << "Unexpected exception: " << ex.what();
    }

    daemon.join();
}

TEST( Service, InterConnect )
{
    using namespace mega::service;
    using namespace mega::test;
    LogicalThread::reset();

    std::promise< void > waitForDaemonStart;
    std::future< void > waitForDaemonStartFut = waitForDaemonStart.get_future();
    std::thread daemon(
        [&waitForDaemonStart]
        {
            try
            {
                Daemon daemon( {}, PortNumber{ 1234 } );
                OConnectivity connectivity(daemon);
                OTestFactory testFactory( daemon );
                waitForDaemonStart.set_value();
                daemon.run();
            }
            catch( Shutdown& ) { }
        });
    waitForDaemonStartFut.get();

    std::promise< void > waitForClient1Start;
    std::future< void > waitForClient1StartFut = waitForClient1Start.get_future();
    std::thread client1(
        [&]
        {
            try
            {
                Connect con( {}, PortNumber{ 1234 } );
                waitForClient1Start.set_value();
                con.run();
            }
            catch( Shutdown& ) { }
        });

    std::promise< void > waitForClient2Start;
    std::future< void > waitForClient2StartFut = waitForClient2Start.get_future();
    std::thread client2(
        [&]
        {
            try
            {
                Connect con( {}, PortNumber{ 1234 } );
                waitForClient2Start.set_value();
                con.run();
            }
            catch( Shutdown& ) { }
        });

    std::promise< void > waitForClient3Start;
    std::future< void > waitForClient3StartFut = waitForClient3Start.get_future();
    std::thread client3(
        [&]
        {
            try
            {
                Connect con( {}, PortNumber{ 1234 } );
                waitForClient3Start.set_value();
                con.run();
            }
            catch( Shutdown& ) { }
        });

    waitForClient1StartFut.get();
    waitForClient2StartFut.get();
    waitForClient3StartFut.get();

    {
        try
        {
            Connect con( {}, PortNumber{ 1234 } );
            auto pConnectivity = con.readRegistry()->one< Connectivity >( MP{} );
            pConnectivity->shutdown();
        }
        catch( Shutdown& ) { }
    }

    client1.join();
    client2.join();
    client3.join();
    daemon.join();
}

TEST( Service, InterClientRequest )
{
    using namespace mega::service;
    using namespace mega::test;
    LogicalThread::reset();

    std::promise< void > waitForDaemonStart;
    std::future< void > waitForDaemonStartFut = waitForDaemonStart.get_future();
    std::thread daemon(
        [&waitForDaemonStart]
        {
            try
            {
                Daemon daemon( {}, PortNumber{ 1234 } );
                OConnectivity connectivity(daemon);
                OTestFactory testFactory( daemon );
                waitForDaemonStart.set_value();
                daemon.run();
            }
            catch( Shutdown& ) { }
        });
    waitForDaemonStartFut.get();

    std::promise< void > waitForClient1Start;
    std::future< void > waitForClient1StartFut = waitForClient1Start.get_future();
    std::thread client1(
        [&]
        {
            try
            {
                Connect con( {}, PortNumber{ 1234 } );
                OTestFactory testFactory( con );
                waitForClient1Start.set_value();
                con.run();
            }
            catch( Shutdown& ) { }
        });


    waitForClient1StartFut.get();

    {
        try
        {
            Connect con( {}, PortNumber{ 1234 } );

            auto pDaemonTestFactory = con.readRegistry()->one< TestFactory >( MP{MachineID{0},ProcessID{0}} );
            auto pDaemonTest = pDaemonTestFactory->create_test();
            ASSERT_EQ( pDaemonTest->test1(), "Hello World"s );

            auto pTestFactory = con.readRegistry()->one< TestFactory >( MP{MachineID{0},ProcessID{1}} );
            auto pTest = pTestFactory->create_test();

            ASSERT_TRUE(pTest);
            // for( int i = 0; i != 1000; i++ )
            {
                ASSERT_EQ( pTest->test1(), "Hello World"s );
                ASSERT_EQ( pTest->test2( 123 ), 123 );
                ASSERT_EQ( pTest->test3( "This"s ), "This"s );
                ASSERT_EQ( pTest->test3( ""s ), ""s );
            }

            ASSERT_EQ( pTest->test4( pDaemonTest ), "Hello World"s );
            ASSERT_EQ( pTest->test4( pTest ), "Hello World"s );

            auto pConnectivity = con.readRegistry()->one< Connectivity >( MP{} );
            pConnectivity->shutdown();
        }
        catch( Shutdown& ) { }
    }

    client1.join();
    daemon.join();

}

