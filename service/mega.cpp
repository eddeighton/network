//  Copyright (c) Deighton Systems Limited. 2022. All Rights Reserved.
//  Author: Edward Deighton
//  License: Please see license.txt in the project root folder.

//  Use and copying of this software and preparation of derivative
//  works based upon this software are permitted. Any copy of this
//  software or of any derivative work must include the above
//  copyright notice, this paragraph and the one after it.  Any
//  distribution of this software or derivative works must comply with
//  all applicable laws.

//  This software is made available AS IS, and COPYRIGHT OWNERS
//  DISCLAIMS ALL WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT
//  LIMITATION THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
//  FOR A PARTICULAR PURPOSE, AND NOTWITHSTANDING ANY OTHER PROVISION
//  CONTAINED HEREIN, ANY LIABILITY FOR DAMAGES RESULTING FROM THE
//  SOFTWARE OR ITS USE IS EXPRESSLY DISCLAIMED, WHETHER ARISING IN
//  CONTRACT, TORT (INCLUDING NEGLIGENCE) OR STRICT LIABILITY, EVEN IF
//  COPYRIGHT OWNERS ARE ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.

#include "test/test_object.hpp"
#include "test/test_factory.hpp"

#include "controller/controller.hpp"
#include "controller/neovim.hpp"

#include "service/connect.hpp"
#include "service/connectivity.hpp"

#include "common/log.hpp"

#include <pybind11/embed.h>

#include <boost/program_options.hpp>
#include <boost/fiber/operations.hpp>

#include <filesystem>
#include <vector>
#include <string>
#include <iostream>
#include <chrono>

using namespace std::string_literals;

int main( int argc, const char* argv[] )
{
    std::optional< std::chrono::steady_clock::time_point >
        startTimeOpt;

    {
        namespace po = boost::program_options;
        po::variables_map vm;

        std::filesystem::path logDir
            = std::filesystem::current_path();
        bool bTime = false;

        mega::service::PortNumber port{ 1234 };
        mega::service::IPAddress  ipAddress{ "localhost"s };
        bool                      bRunAsServer = false;

        std::vector< std::string > commandArgs;

        po::options_description genericOptions( " General" );
        {
            // clang-format off
            genericOptions.add_options()
            ( "help,?",                                                         "Produce general or command help message" )
            ( "log_dir",    po::value< std::filesystem::path >( &logDir ),      "Build log directory" )
            ( "time",       po::bool_switch( &bTime ),                          "Measure time taken to perform command" )
            ( "server,s",   po::bool_switch( &bRunAsServer ),                   "Run as server" )
            ( "ip,i",       po::value( &ipAddress ),                            "Daemon IP Address" )
            ( "port,p",     po::value( &port ),                                 "Daemon Port Number" )
            ;
            // clang-format on
        }

        std::string strCommand;

        po::options_description commandOptions( " Commands" );
        {
            commandOptions.add_options()(
                "cmd,c",
                po::value< std::string >( &strCommand ),
                "Python base command to execute" );
        }

        // capture vector of strings as hidden args option which
        // can be forwarded to nested command
        po::options_description commandHiddenOptions( "" );
        {
            commandHiddenOptions.add_options()(
                "args",
                po::value< std::vector< std::string > >(
                    &commandArgs ) );
        }

        po::options_description visibleOptions( "Allowed options" );
        visibleOptions.add( genericOptions ).add( commandOptions );

        po::options_description allOptions( "all" );
        allOptions.add( genericOptions )
            .add( commandOptions )
            .add( commandHiddenOptions );

        po::positional_options_description p;
        p.add( "args", -1 );

        po::parsed_options parsedOptions
            = po::command_line_parser( argc, argv )
                  .options( allOptions )
                  .positional( p )
                  .allow_unregistered()
                  .run();
        po::store( parsedOptions, vm );
        po::notify( vm );

        try
        {
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
                mega::service::Connect connection( ipAddress, port );

                mega::test::OTestFactory testFactory( connection );

                mega::controller::OController controller( connection );
                mega::controller::ONeovimFactory neovimFactor( connection );

                if( !strCommand.empty() )
                {
                    LOG( "Got command: " << strCommand );

                    boost::fibers::fiber(
                        [ & ]()
                        {
                            pybind11::scoped_interpreter guard{};
                            // pybind11::module_ megastructureModule =
                            //     pybind11::module_::import("megastructure");

                            using namespace pybind11::literals;

                            std::ostringstream os;
                            os << "import megastructure as mega\n";
                            os << strCommand;

                            pybind11::exec(
                                os.str(), pybind11::globals() );
                        } )
                        .detach();
                }

                if( bRunAsServer )
                {
                    connection.run();
                }
            }
        }
        catch( std::exception& ex )
        {
            LOG( "Exception: " << ex.what() );
            return 1;
        }
    }

    return 0;
}
