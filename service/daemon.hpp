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

#include "service/router.hpp"
#include "service/server.hpp"
#include "service/client.hpp"
#include "service/asio.hpp"
#include "service/enrole.hpp"

#include "service/protocol/serialization.hpp"
#include "service/protocol/message_factory.hpp"

#include "vocab/service/mp.hpp"

#include <thread>
#include <atomic>
#include <tuple>

namespace mega::service
{
    class Daemon
    {
        MP                          m_mp;
        PortNumber                  m_port;
        IOContextPtr                m_pIOContext;
        std::atomic<bool>           m_bShutdown{false};
        std::atomic<bool>           m_bStopped{false};
        std::unique_ptr< Server >   m_pServer;
        std::unique_ptr< Client >   m_pClient;
        std::thread                 m_thread;
        Router::Table               m_connectionsTable;
        Router::Map                 m_routers;
        Registration                m_registration;

    private:
        void route(MessageType messageType,
                   const Header& header, 
                   const PacketBuffer& buffer,
                   Connection::WeakPtr pResponseConnection ) 
        {
            auto iFind = m_routers.find( header.m_stack );
            if( iFind == m_routers.end() )
            {
                bool bSuccess{};
                std::tie( iFind, bSuccess ) = 
                    m_routers.insert
                    ( 
                        {
                            header.m_stack,
                            std::make_unique< Router >
                            (
                                header.m_stack,
                                m_connectionsTable
                            )
                        }
                    );
                VERIFY_RTE_MSG( bSuccess, "Failed to allocate router" );
            }
            iFind->second->send(
                Router::ServiceMessage
                {
                    messageType,
                    header,
                    buffer,
                    pResponseConnection
                }
            );
        }

    public:
        Daemon(MP mp, PortNumber port )
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
                        [this](Connection::WeakPtr pResponseConnection,
                                const PacketBuffer& buffer)
                        { 
                            receive( pResponseConnection, buffer ); 
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
                        [this](Connection::WeakPtr pResponseConnection,
                                const PacketBuffer& buffer)
                        { 
                            receive( pResponseConnection, buffer ); 
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

                    init_fiber_scheduler(m_pIOContext);
                    m_pIOContext->run();
                }
                std::cout << "network thread shutting down" << std::endl;
            })
        {
            LogicalThread::registerFiber(m_mp);

            mega::service::Registry::getWriteAccess()->setCreationCallback(
                [&cons = m_connectionsTable, mp = m_mp, &registration = m_registration]()
                {
                    const auto reg = Registry::getReadAccess()->getRegistration();
                    registration.add( reg );

                    for( auto& [ mp, pWeak ] : cons.m_direct )
                    {
                        if( auto pCon = pWeak.lock() )
                        {
                            sendRegistration( registration, { mp }, pCon->getSender() );
                        }
                    }
                });
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
            LogicalThread& thisLogicalThread
                = LogicalThread::get();

            while( m_bShutdown.load(std::memory_order_relaxed) == false )
            {
                thisLogicalThread.receive();
            }
        }

        void shutdown()
        {
            m_bShutdown = true;
        }

        void receive(Connection::WeakPtr pResponseConnection,
                            const PacketBuffer& buffer)
        {
            boost::interprocess::basic_vectorbuf< PacketBuffer > vectorBuffer(buffer);
            boost::archive::binary_iarchive ia(vectorBuffer, boostArchiveFlags);

            MessageType messageType;
            ia >> messageType;

            switch( messageType )
            {
                case MessageType::eEnrole         :
                    {
                        THROW_RTE( "Unexpected enrole request received" );
                    }
                    break;
                case MessageType::eRegistry        :
                    {
                        std::set< MP > visited;
                        Registration reg;

                        ia >> visited;
                        ia >> reg;

                        visited.insert(m_mp);
                        m_registration.add(reg);
                        reg.add(m_registration);

                        for( auto& [ mp, pWeak ] : m_connectionsTable.m_direct )
                        {
                            if( !visited.contains( mp ) )
                            {
                                if( auto pCon = pWeak.lock() )
                                {
                                    sendRegistration( reg, visited, pCon->getSender() );
                                }
                            }
                        }
                    }
                    break;
                case MessageType::eRequest        :
                    {
                        Header header;
                        ia >> header;
                        
                        if( header.m_responder.getMP() == m_mp )
                        {
                            decodeInboundRequest(ia, header, pResponseConnection);
                        }
                        else
                        {
                            route(messageType, header, buffer, pResponseConnection);
                        }
                    }
                    break;
                case MessageType::eResponse        :
                    {
                        Header header;
                        ia >> header;

                        if( header.m_responder.getMP() == m_mp )
                        {
                            LogicalThread& logicalThread =
                                Registry::getReadAccess()->
                                    getLogicalThread(header.m_requester);

                            logicalThread.send(
                                InterProcessResponse
                                {
                                    header,
                                    buffer
                                }
                            );
                        }
                        else
                        {
                            route(messageType, header, buffer, pResponseConnection);
                        }
                    }
                    break;
                case MessageType::TOTAL_MESSAGES   :
                default:
                    {
                        THROW_RTE(" Unepxcted message type recieved" );
                    }
                    break;
            }
        }

        void connection(Connection::Ptr pConnection)
        {
            const MP mp{ m_mp.getMachineID(), pConnection->getProcessID() };

            // enrole connection
            // std::cout << "Connection callback called for: " <<
            //    pConnection->getSocketInfo() << std::endl;
            m_connectionsTable.m_direct.insert( { mp, pConnection } );

            auto& sender = pConnection->getSender();

            sendEnrole( Enrole{ m_mp, mp }, sender );
            sendRegistration( Registry::getReadAccess()->getRegistration(), { m_mp }, sender );
        }

        void disconnect(Connection::Ptr pConnection)
        {
            const MP mp{ m_mp.getMachineID(), pConnection->getProcessID() };
            m_connectionsTable.m_direct.erase( mp );
        }
    };
}


