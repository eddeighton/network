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

#pragma once

#include "service/server.hpp"
#include "service/client.hpp"
#include "service/asio.hpp"
#include "service/enrole.hpp"
#include "service/protocol/serialization.hpp"

#include "vocab/service/mp.hpp"

#include <thread>
#include <atomic>

namespace mega::service
{
    class Daemon
    {
        MP                          m_mp;
        mega::service::PortNumber   m_port;
        IOContextPtr                m_pIOContext;
        std::atomic<bool>           m_bShutdown{false};
        std::atomic<bool>           m_bStopped{false};
        std::unique_ptr< Server >   m_pServer;
        std::unique_ptr< Client >   m_pClient;
        std::thread                 m_thread;
    public:
        Daemon(MP mp, mega::service::PortNumber port )
        :   m_mp( mp )
        ,   m_port( port )
        ,   m_pIOContext(std::make_shared< boost::asio::io_context >())
        ,   m_thread( [this, port]()
            {
                {
                    m_pServer = std::make_unique< Server >( 
                        *m_pIOContext,
                        port,
                        // receive callback
                        [this](mega::service::SocketSender& responseSender,
                                const mega::service::PacketBuffer& buffer)
                        { 
                            receive( responseSender, buffer ); 
                        },
                        // connection callback
                        [this](Connection::Ptr pConnection)
                        {
                            connection( pConnection );
                        },
                        // disconnect callback
                        [this](Connection::Ptr pConnection)
                        {
                            disconnect( pConnection );
                        }
                    );

                    m_pClient = std::make_unique< Client >(
                        *m_pIOContext,
                        // receive callback
                        [this](mega::service::SocketSender& responseSender,
                                const mega::service::PacketBuffer& buffer)
                        { 
                            receive( responseSender, buffer ); 
                        }
                    );

                    mega::service::init_fiber_scheduler(m_pIOContext);
                    m_pIOContext->run();
                }
                std::cout << "network thread shutting down" << std::endl;
            })
        {
            mega::service::LogicalThread::registerFiber(m_mp);
        }

        ~Daemon()
        {
            boost::fibers::promise<void>    waitForServerShutdown;
            boost::fibers::future<void>     waitForServerShutdownFuture =
                waitForServerShutdown.get_future();
            
            m_pIOContext->post( [&pClient = m_pClient, &pServer = m_pServer, &waitForServerShutdown]()
                {
                    // must avoid blocking asio while
                    // shutting down the server since the
                    // sockets need to return for each
                    // connection to close
                    boost::fibers::fiber( [&pClient, &pServer, &waitForServerShutdown]()
                    {
                        pClient->stop();
                        pServer->stop();
                        waitForServerShutdown.set_value();
                        std::cout << "Server stop complete" << std::endl;
                    }).detach();
                });

            waitForServerShutdownFuture.get();
 
            m_pIOContext->stop();

            std::cout << "io service stopped" << std::endl;

            m_thread.join();

            std::cout << "Daemon shut down" << std::endl;
        }

        void run()
        {
            // run this logical thread while network running
            mega::service::LogicalThread& thisLogicalThread
                = mega::service::LogicalThread::get();

            while( m_bShutdown.load(std::memory_order_relaxed) == false )
            {
                thisLogicalThread.receive();
            }
        }

        void shutdown()
        {
            m_bShutdown = true;
        }

        void receive(mega::service::SocketSender& responseSender,
                            const mega::service::PacketBuffer& buffer)
        {
            boost::interprocess::basic_vectorbuf< mega::service::PacketBuffer > vectorBuffer(buffer);
            boost::archive::binary_iarchive ia(vectorBuffer, mega::service::boostArchiveFlags);

            mega::service::MessageType messageType;
            ia >> messageType;

            switch( messageType )
            {
                case mega::service::MessageType::eEnrole         :
                    {
                        THROW_RTE( "Unexpected enrole request received" );
                    }
                    break;
                case mega::service::MessageType::eRegistry        :
                    {
                    }
                    break;
                case mega::service::MessageType::eRequest        :
                    {
                        mega::service::Header header;
                        ia >> header;
                       
                        if( header.m_responder.getMP() == m_mp )
                        {
                            mega::service::decodeInboundRequest(ia, header, responseSender);
                        }
                        else
                        {
                            // dispatch message to router...
                        }
                    }
                    break;
                case mega::service::MessageType::eResponse        :
                    {
                        mega::service::Header header;
                        ia >> header;

                        mega::service::LogicalThread& logicalThread =
                            mega::service::Registry::getReadAccess()->
                                getLogicalThread(header.m_requester);

                        logicalThread.send(
                            mega::service::InterProcessResponse
                            {
                                header,
                                buffer
                            }
                        );
                    }
                    break;
                case mega::service::MessageType::TOTAL_MESSAGES   :
                default:
                    {
                        THROW_RTE(" Unepxcted message type recieved" );
                    }
                    break;
            }
        }

        void connection(Connection::Ptr pConnection)
        {
            // enrole connection
            // std::cout << "Connection callback called for: " <<
            //    pConnection->getSocketInfo() << std::endl;

            auto& sender = pConnection->getSender();

            // send enrole message
            {
                boost::interprocess::basic_vectorbuf< mega::service::PacketBuffer > vectorBuffer;
                boost::archive::binary_oarchive oa(vectorBuffer, boostArchiveFlags);

                oa << mega::service::MessageType::eEnrole;

                mega::service::Enrole enrole
                {
                    m_mp,
                    MP{ m_mp.getMachineID(), pConnection->getProcessID() }
                };

                oa << enrole;

                sender.send(vectorBuffer.vector());
            }
            {
                // send registration
                boost::interprocess::basic_vectorbuf< mega::service::PacketBuffer > vectorBuffer;
                boost::archive::binary_oarchive oa(vectorBuffer, boostArchiveFlags);

                oa << mega::service::MessageType::eRegistry;

                const auto registration = 
                    mega::service::Registry::getReadAccess()->getRegistration();

                oa << registration;

                sender.send(vectorBuffer.vector());
            }
        }

        void disconnect(Connection::Ptr pConnection)
        {
            
        }


    };
}


