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

#include "service/asio.hpp"
#include "service/server.hpp"

#include "service/logical_thread.hpp"

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

        namespace po = boost::program_options;
        po::options_description commandOptions( " Execute inja template" );
        {
            // clang-format off
            commandOptions.add_options()
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
     
            mega::service::IOContextPtr pIOContext =
                std::make_shared< boost::asio::io_context >();

            std::thread networkThread(
                [&]
                {                    
                    mega::service::init_fiber_scheduler(pIOContext);
                    pIOContext->run();
                }
            );

            mega::service::MP mp
            { 
                mega::service::MachineID{0}, 
                mega::service::ProcessID{0}
            };
    
            mega::service::LogicalThread::registerFiber(mp);

            mega::service::ReceiverCallback receiverCallback = 
                []( mega::service::SocketSender& responseSender,
                    const mega::service::PacketBuffer& buffer)
                {
                    static constexpr auto boostArchiveFlags = boost::archive::no_header | boost::archive::no_codecvt
                                      | boost::archive::no_xml_tag_checking | boost::archive::no_tracking;
                    boost::interprocess::basic_vectorbuf< mega::service::PacketBuffer > vectorBuffer(buffer);
                    boost::archive::binary_iarchive ia(vectorBuffer, boostArchiveFlags);

                    mega::service::Header header;
                    ia >> header;
                    std::cout << "Got packet: " << header << std::endl;
                };

            mega::service::Server server(*pIOContext, port, std::move(receiverCallback));


            mega::service::LogicalThread::get().runMessageLoop();
            networkThread.join();
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

