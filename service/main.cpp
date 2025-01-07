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

#include "service/daemon.hpp"
#include "service/connectivity.hpp"
#include "test/test_factory.hpp"

#include <boost/program_options.hpp>

#include <iostream>
#include <filesystem>
#include <memory>

int main( int argc, const char* argv[] ) 
{
    {
        namespace po = boost::program_options;
        po::variables_map vm;
        bool              bGeneralWait = false;

        po::options_description genericOptions( " General" );
        {
            // clang-format off
            genericOptions.add_options()
            ( "help,?",                                          "Produce general or command help message" )
            ( "wait",       po::bool_switch( &bGeneralWait ),    "Wait at startup for attaching a debugger" );
            // clang-format on
        }

        mega::service::PortNumber port{1234};
        mega::service::MachineID machineID{};

        namespace po = boost::program_options;
        po::options_description commandOptions( " Daemon Configuration" );
        {
            // clang-format off
            commandOptions.add_options()
            ( "machine,m",  po::value< mega::service::MachineID >( &machineID ),
              "Four byte machine ID by which this daemon will be know.  MUST be unique!" )
            ( "port,p",       po::value< mega::service::PortNumber >( &port ),
              "Port number daemon should bind to on localhost" )
                ;
            // clang-format on
        }

        po::options_description visibleOptions( "Allowed options" );
        visibleOptions.add( genericOptions ).add( commandOptions );

        po::parsed_options parsedOptions = po::command_line_parser( argc, argv ).options( visibleOptions ).run();
        po::store( parsedOptions, vm );
        po::notify( vm );

        if( vm.count( "help" ) )
        {
            std::cout << visibleOptions << "\n";
            return 0;
        }
        try
        {
            if( bGeneralWait )
            {
                std::cout << "Waiting for input..." << std::endl;
                char c;
                std::cin >> c;
            }
     
            const mega::service::MP mp
            { 
                machineID, 
                mega::service::PROCESS_ZERO
            };

            mega::service::Daemon daemon(mp, port);
            mega::service::OConnectivity connectivity(daemon);
            mega::test::OTestFactory testFactory(daemon);

            daemon.run();

            std::cout << "main shutting down" << std::endl;
        }
        catch( std::exception& e )
        {
            std::cout << "Exception: " << e.what() << std::endl;
            return 1;
        }
        catch( ... )
        {
            std::cout << "Unknown error" << std::endl;
            return 1;
        }
    }
    
    return 0;
}

