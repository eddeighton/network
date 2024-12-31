#include "test/test_object.hpp"
#include "test/test_factory.hpp"

#include "service/logical_thread.hpp"
#include "service/network.hpp"
#include "service/registry.hpp"
#include "service/ptr.hpp"

#include <boost/process.hpp>
#include <boost/asio/io_service.hpp>

#include <gtest/gtest.h>

#include <utility>
#include <iostream>

TEST( Service, Daemon )
{
    using namespace mega::service;
    using namespace mega::test;

    
    namespace bp = boost::process;

    std::future< std::string > daemonOutput, daemonError;
    std::future< std::string > megaOutput,   megaError;

    boost::asio::io_service ios;

    bp::child procDaemon(   "/build/debug/bin/daemon",  bp::std_in.close(), bp::std_out > daemonOutput, bp::std_err > daemonError, ios );
    bp::child procMega(     "/build/debug/bin/mega",    bp::std_in.close(), bp::std_out > megaOutput, bp::std_err > megaError, ios );

    ios.run();

    auto strDaemonOut = daemonOutput.get();
    auto strMegaOut  = megaOutput.get();

    std::cout << strDaemonOut << std::endl;
    std::cout << strMegaOut << std::endl;
}

