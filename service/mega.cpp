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

#include "service/gen/daemon.proxy.hxx"

#include "test/test_object.hpp"
#include "test/test_factory.hpp"

#include "service/client.hpp"
#include "service/network.hpp"
#include "service/enrole.hpp"

#include "service/protocol/message.hpp"

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
    std::optional< std::chrono::steady_clock::time_point > startTimeOpt;

    {
        namespace po = boost::program_options;
        po::variables_map vm;

        std::filesystem::path logDir = std::filesystem::current_path();
        bool                  bGeneralWait = false;
        bool                  bTime = false;

        mega::service::PortNumber port{1234};
        mega::service::IPAddress  ipAddress{"localhost"s};

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

                mega::service::Enrole enrolement;

                boost::fibers::promise<void> waitForRegistryPromise;
                boost::fibers::future<void> registrationFuture =
                    waitForRegistryPromise.get_future();
     
                mega::service::ReceiverCallback receiverCallback = 
                    [ &waitForRegistryPromise, &enrolement ]( 
                        mega::service::SocketSender& responseSender,
                        const mega::service::PacketBuffer& buffer)
                    {
                        static constexpr auto boostArchiveFlags =
                            boost::archive::no_header
                          | boost::archive::no_codecvt
                          | boost::archive::no_xml_tag_checking
                          | boost::archive::no_tracking;

                        boost::interprocess::basic_vectorbuf
                            < mega::service::PacketBuffer > vectorBuffer(buffer);
                        boost::archive::binary_iarchive ia(vectorBuffer, boostArchiveFlags);

                        mega::service::MessageType messageType;
                        ia >> messageType;

                        switch( messageType )
                        {
                            case mega::service::MessageType::eEnrole         :
                                {                                   
                                    ia >> enrolement;
                                    std::cout << "Got enrolement from daemon: " 
                                        << enrolement.getDaemon() << " with mp: "
                                        << enrolement.getMP() << std::endl; 
                                }
                                break;
                            case mega::service::MessageType::eRegistry        :
                                {
                                    std::cout << "Got registration update" << std::endl;
                                    mega::service::Registration registration;
                                    ia >> registration;
                                    mega::service::Registry::getWriteAccess()->update(
                                        responseSender, registration);
                                    waitForRegistryPromise.set_value();
                                }
                                break;
                            case mega::service::MessageType::eConnect         :
                                {
                                }
                                break;
                            case mega::service::MessageType::eDisconnect      :
                                {
                                }
                                break;
                            case mega::service::MessageType::eRoute           :
                                {
                                }
                                break;
                            case mega::service::MessageType::eShutdown        :
                                {
                                }
                                break;
                            case mega::service::MessageType::eRequest        :
                                {
                                    mega::service::Header header;
                                    ia >> header;
                                    // std::cout << "Got request: " << header << std::endl;
                                }
                                break;
                            case mega::service::MessageType::eResponse        :
                                {
                                    mega::service::Header header;
                                    ia >> header;
                                    // std::cout << "Got response: " << header << std::endl;

                                    mega::service::LogicalThread& logicalThread = [&]() -> mega::service::LogicalThread&
                                    {
                                        auto reg = mega::service::Registry::getReadAccess();
                                        return reg->getLogicalThread(header.m_requester);
                                    }();

                                    logicalThread.send(
                                        mega::service::InterProcessResponse
                                        {
                                            header,
                                            {}
                                        }
                                    );
                                }
                                break;
                            case mega::service::MessageType::TOTAL_MESSAGES   :
                                {
                                    std::string strMsg;
                                    ia >> strMsg;
                                    std::cout << "Got packet: " << strMsg << std::endl;
                                }
                                break;
                        }
                    };

                mega::service::Client client(network, std::move(receiverCallback));

                auto pConnection = client.connect(ipAddress, port);

                // wait for enrolement and registration to complete
                registrationFuture.get();

                mega::service::LogicalThread::registerFiber(enrolement.getMP());

                using namespace mega::service;
                using namespace mega::test;
                auto pDaemonConnectivity = 
                    Registry::getReadAccess()->one< Connectivity >(
                        enrolement.getDaemon()
                    );

                pDaemonConnectivity->shutdown();

                pConnection->stop();

                // if( runAsServer )
                // {
                // mega::service::LogicalThread::get().runMessageLoop();
                // }
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

