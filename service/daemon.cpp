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

#include "service/connectivity.hpp"

#include "service/asio.hpp"
#include "service/server.hpp"
#include "service/network.hpp"
#include "service/logical_thread.hpp"
#include "service/enrole.hpp"

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
            ( "MachineID,m",  po::value< mega::service::MachineID >( &machineID ),      "Four byte machine ID by which this daemon will be know.  MUST be unique!" )
            ( "Port,p",       po::value< mega::service::PortNumber >( &port ),          "Port number daemon should bind to on localhost" )
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

            mega::service::Network network;
 
            mega::service::LogicalThread::registerFiber(mp);

            mega::service::ReceiverCallback receiverCallback = 
                [&]( mega::service::SocketSender& responseSender,
                    const mega::service::PacketBuffer& buffer)
                {
                    static constexpr auto boostArchiveFlags = 
                        boost::archive::no_header | boost::archive::no_codecvt
                                      | boost::archive::no_xml_tag_checking | boost::archive::no_tracking;
                    boost::interprocess::basic_vectorbuf< mega::service::PacketBuffer > vectorBuffer(buffer);
                    boost::archive::binary_iarchive ia(vectorBuffer, boostArchiveFlags);

                    mega::service::Header header;
                    ia >> header;
                    std::cout << "Got packet: " << header << std::endl;

                    const auto& objectInfo = [&]()
                    {
                        auto reg = mega::service::Registry::getReadAccess();
                        return reg->getObjectInfo(header.m_responder);
                    }();

                    // request or response
                    objectInfo.logicalThread.send(
                        mega::service::InterProcessRequest
                        {
                            [&]()
                            {
                                auto p = dynamic_cast<mega::test::Connectivity*>(objectInfo.pObject);
                                p->shutdown();

                                mega::service::PacketBuffer responseBuffer;
                                // responseSender.send(responseBuffer);
                            }
                        }
                    );
                };

            mega::service::ProcessID nextMegaProcessID{ 1 };

            mega::service::Server server(network, port, std::move(receiverCallback),
                [&](mega::service::Server::Connection::Ptr pConnection)
                {
                    // enrole connection
                    std::cout << "Connection callback called for: " <<
                        pConnection->getSocketInfo() << std::endl;

                    auto& sender = pConnection->getSender();

                    // send enrole message
                    static constexpr auto boostArchiveFlags = 
                        boost::archive::no_header | boost::archive::no_codecvt
                                      | boost::archive::no_xml_tag_checking | boost::archive::no_tracking;
                    boost::interprocess::basic_vectorbuf< mega::service::PacketBuffer > vectorBuffer;
                    boost::archive::binary_oarchive oa(vectorBuffer, boostArchiveFlags);

                    mega::service::Enrole enrole{ nextMegaProcessID++ };

                    oa << enrole;

                    sender.send(vectorBuffer.vector());
                });

            mega::test::OConnectivity connectivity;

            mega::service::LogicalThread::get().runMessageLoop();
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

