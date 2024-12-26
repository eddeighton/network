//  Copyright (c) Deighton Systems Limited. 2022. All Rights Reserved.
//  Author: Edward Deighton
//  License: Please see license.txt in the project root folder.

//  Use and copying of this software and preparation of derivative works
//  based upon this software are permitted. Any copy of this software or
//  of any derivative work must include the above copyright notice, this
//  paragraph and the one after it.  Any distribution of this software or
//  derivative works must comply with all applicable laws.

//  This software is made available AS IS, and COPYRIGHT OWNERS DISCLAIMS
//  ALL WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE
//  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
//  PURPOSE, AND NOTWITHSTANDING ANY OTHER PROVISION CONTAINED HEREIN, ANY
//  LIABILITY FOR DAMAGES RESULTING FROM THE SOFTWARE OR ITS USE IS
//  EXPRESSLY DISCLAIMED, WHETHER ARISING IN CONTRACT, TORT (INCLUDING
//  NEGLIGENCE) OR STRICT LIABILITY, EVEN IF COPYRIGHT OWNERS ARE ADVISED
//  OF THE POSSIBILITY OF SUCH DAMAGES.

#include "service/client.hpp"
#include "service/network.hpp"

#include <boost/program_options.hpp>
#include <boost/fiber/operations.hpp>

#include <filesystem>
#include <vector>
#include <string>
#include <iostream>
#include <chrono>

namespace mega::test
{
    extern void runTestComponent(service::Network& network);
}

int main( int argc, const char* argv[] )
{
    std::optional< std::chrono::steady_clock::time_point > startTimeOpt;

    {
        namespace po = boost::program_options;
        po::variables_map vm;

        std::filesystem::path logDir = std::filesystem::current_path();
        bool                  bGeneralWait = false;
        bool                  bTime = false;

        std::vector<std::string> commandArgs;

        po::options_description genericOptions( " General" );
        {
            // clang-format off
            genericOptions.add_options()
            ( "help,?",                                                         "Produce general or command help message" )
            ( "log_dir",    po::value< std::filesystem::path >( &logDir ),      "Build log directory" )
            ( "wait",       po::bool_switch( &bGeneralWait ),                   "Wait at startup for attaching a debugger" )
            ( "time",       po::bool_switch( &bTime ),                          "Measure time taken to perform command" );
            // clang-format on
        }


        po::options_description commandOptions( " Commands" );
        {
            commandOptions.add_options()
                ;
        }

        // capture vector of strings as hidden args option which
        // can be forwarded to nested command
        po::options_description commandHiddenOptions( "" );
        {
            commandHiddenOptions.add_options()( "args", po::value< std::vector< std::string > >( &commandArgs ) );
        }

        po::options_description visibleOptions( "Allowed options" );
        visibleOptions.add( genericOptions ).add( commandOptions );

        po::options_description allOptions( "all" );
        allOptions.add( genericOptions ).add( commandOptions ).add( commandHiddenOptions );

        po::positional_options_description p;
        p.add( "args", -1 );

        po::parsed_options parsedOptions
            = po::command_line_parser( argc, argv ).options( allOptions ).positional( p ).allow_unregistered().run();
        po::store( parsedOptions, vm );
        po::notify( vm );

        try
        {
            if( bGeneralWait )
            {
                std::cout << "Waiting for input..." << std::endl;
                char c;
                std::cin >> c;
            }

            if( bTime )
            {
                startTimeOpt = std::chrono::steady_clock::now();
            }


            const bool bShowHelp = vm.count( "help" );

            if( bShowHelp )
            {
                std::cout << visibleOptions << "\n";
            }
            else
            {

                mega::service::Network network;

                boost::fibers::buffered_channel< int > channel(2);
                boost::fibers::buffered_channel< int > channel2(2);

                mega::test::runTestComponent(network);

                std::thread runAnotherThread( [&]
                    {
                        //boost::this_fiber::yield();
                        //pIOContext->run();
                        int itest=0;
                        auto r = channel2.pop(itest);
                        if( itest == 1 )
                        {
                            throw 123;
                        }
                    });
                // boost::this_fiber::yield();

                // pIOContext->run();
                int itest=0;
                auto r = channel.pop(itest);
                if( itest == 1 )
                {
                    throw 123;
                }
                

                // runIOService.join();

                // std::thread networkThread(
                //     []
                //     {
                //         mega::service::IOContextPtr pIOContext =
                //             std::make_shared< boost::asio::io_context >();


                //         mega::service::Client client(*pIOContext);

                //         using namespace mega::service;
                //         using namespace std::string_literals;

                //         boost::fibers::fiber([&]
                //             {
                //                 auto pConnection = client.connect(IPAddress{"localhost"}, PortNumber{1234});
                //                 pConnection->getSender().send("ONE "s);
                //                 pConnection->getSender().send("TWO "s);
                //                 pConnection->getSender().send("THRE"s);
                //                 pConnection->getSender().send("FOUR"s);
                //                 pConnection->stop();
                //                 pIOContext->run_one();
                //                 pIOContext->stop();
                //             }).detach();

                //         pIOContext->run();

                //         std::cout << "Complete" << std::endl;
                //     }
                // );

                // networkThread.join();
            }
        }
        catch(std::exception& ex)
        {
            std::cout << "Exception: " << ex.what() << std::endl;
            return 1;
        }
    }

    return 0;
}

