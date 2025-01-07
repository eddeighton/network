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
#include "service/access.hpp"
#include "service/asio.hpp"
#include "service/enrole.hpp"

#include "service/protocol/serialization.hpp"
#include "service/protocol/message_factory.hpp"

#include "vocab/service/mp.hpp"

#include "common/log.hpp"

#include <thread>
#include <atomic>
#include <tuple>

namespace mega::service
{
    class Daemon : public Access
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
                //LOG( "network thread shutting down" ) ;
            })
        {
            LogicalThread::registerFiber(m_mp);

            writeRegistry()->setCreationCallback(
                [&cons = m_connectionsTable, mp = m_mp, &registration = m_registration](Registration reg)
                {
                    registration.add( reg );
                    //LOG( "Generated reg update of: " << registration ) ;
                    for( auto& [ mp, pWeak ] : cons.getDirect() )
                    {
                        if( auto pCon = pWeak.lock() )
                        {
                            //LOG( "Sending reg update to: " << mp ) ;
                            sendRegistration( registration, { mp }, pCon->getSender() );
                        }
                    }
                });

            // ensure server is started so that tests work
            auto pFut = m_pIOContext->post(
                boost::asio::use_future([this]()
                {
                    m_pServer->start();
                }));
            pFut.get();
        }

        ~Daemon()
        {
            m_bShutdown = true;

            boost::fibers::promise<void>    waitForServerShutdown;
            boost::fibers::future<void>     waitForServerShutdownFuture =
                waitForServerShutdown.get_future();

            Router::Map routers = std::move( m_routers );
            
            m_pIOContext->post(
                [ 
                    &pClient = m_pClient,
                    &pServer = m_pServer,
                    &waitForServerShutdown,
                    routers = std::move( routers )
                ]() mutable
                {
                    // must avoid blocking asio while
                    // shutting down the server since the
                    // sockets need to return for each
                    // connection to close
                    boost::fibers::fiber(
                        [
                            &pClient, 
                            &pServer, 
                            &waitForServerShutdown,
                            routers = std::move( routers )
                        ]() mutable
                    {
                        LOG( "Daemon shutdown fiber start" ) ;
                        pClient->stop();
                        pServer->stop();
                        routers.clear();
                        waitForServerShutdown.set_value();
                        //LOG( "Server stop complete" ) ;
                        LOG( "Daemon shutdown fiber stop" ) ;
                    }).detach();
                });

            waitForServerShutdownFuture.get();
 
            m_pIOContext->stop();

            //LOG( "io service stopped" ) ;

            m_thread.join();

            //LOG( "Daemon shut down" ) ;
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
            IArchive ia(*this, buffer);

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

                        //LOG( "Got reg update: " << reg ) ;

                        for( auto& [ mp, pWeak ] : m_connectionsTable.getDirect() )
                        {
                            if( !visited.contains( mp ) )
                            {
                                if( auto pCon = pWeak.lock() )
                                {
                                    //LOG( "Forwarding reg to: " << mp ) ;
                                    sendRegistration( reg, visited, pCon->getSender() );
                                }
                            }
                        }
                    }
                    break;
                case MessageType::eDisconnect      :
                    {
                        std::set< MP > visited;
                        MP shutdownMP;

                        ia >> visited;
                        ia >> shutdownMP;

                        writeRegistry()->disconnected(shutdownMP);
                        m_registration.remove(shutdownMP);

                        for( auto& [ mp, pWeak ] : m_connectionsTable.getDirect() )
                        {
                            if( !visited.contains( mp ) )
                            {
                                if( auto pCon = pWeak.lock() )
                                {
                                    //LOG( "Forwarding reg to: " << mp ) ;
                                    sendDisconnect( shutdownMP, visited, pCon->getSender() );
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
                            decodeInboundRequest(*this, header, buffer, pResponseConnection);
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
                            LogicalThread::get(header.m_requester).send(
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
            // LOG( "Connection callback called for: " <<
            //    pConnection->getSocketInfo() ) ;
            m_connectionsTable.addDirect( mp, pConnection );

            auto& sender = pConnection->getSender();

            sendEnrole( Enrole{ m_mp, mp }, sender );
            //LOG( "Sending registration: " << m_registration ) ;
            sendRegistration( m_registration, { m_mp }, sender );
        }

        void disconnect(Connection::Ptr pConnection)
        {
            const MP mp{ m_mp.getMachineID(), pConnection->getProcessID() };
            m_connectionsTable.removeDirect(mp);

            std::set< MP > visited{m_mp};

            writeRegistry()->disconnected(mp);
            m_registration.remove(mp);

            for( auto& [ mp, pWeak ] : m_connectionsTable.getDirect() )
            {
                if( auto pCon = pWeak.lock() )
                {
                    //LOG( "Forwarding reg to: " << mp ) ;
                    sendDisconnect( mp, visited, pCon->getSender() );
                }
            }
        }
    };
}


