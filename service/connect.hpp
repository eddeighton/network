
#pragma once

#include "service/client.hpp"
#include "service/access.hpp"
#include "service/registry.hpp"
#include "service/enrole.hpp"

#include "service/gen/decoder.hxx"

#include "service/protocol/message.hpp"
#include "service/protocol/serialization.hpp"
#include "service/protocol/message_factory.hpp"

#include "common/log.hpp"

#include <boost/fiber/operations.hpp>

#include <vector>
#include <functional>
#include <optional>

// disable logging
// #define LOG_CONNECT( msg )
using namespace std::string_literals;
#define LOG_CONNECT( msg ) LOG( "CONNECT: "s + msg )

namespace mega::service
{
class Connect : public Access
{
    IOContextPtr                   m_pIOContext;
    std::unique_ptr< Client >      m_pClient;
    boost::fibers::promise< void > m_startupPromise;
    boost::fibers::future< void >  m_startupFuture;
    bool                           m_bStarting = true;
    std::thread                    m_thread;
    Enrole                         m_enrolement;
    Client::Connection::WeakPtr    m_pConnection;
    std::atomic< bool >            m_bShuttingDown{ false };

public:
    Connect( IPAddress ipAddress, PortNumber port )
        : m_pIOContext(
              std::make_shared< boost::asio::io_context >() )
        , m_startupFuture( m_startupPromise.get_future() )
        , m_thread(
              [ this, ipAddress, port ]()
              {
                  LogicalThread::registerThread();
                  {
                      m_pClient = std::make_unique< Client >(
                          *m_pIOContext,
                          // receive callback
                          [ this ](
                              Connection::WeakPtr pResponseConnection,
                              const PacketBuffer& buffer )
                          {
                              receiverCallback(
                                  pResponseConnection, buffer );
                          },
                          // connection callback
                          [ this ]( Connection::Ptr pConnection )
                          { connectCallback( pConnection ); },
                          // disconnect callback
                          [ this ]( Connection::Ptr pConnection )
                          { disconnectCallback( pConnection ); } );

                      init_fiber_scheduler( m_pIOContext );
                      m_pIOContext->run();
                  }
                  LOG_CLIENT( "network thread shutting down" );
              } )
    {
        LogicalThread::registerThread();

        LOG_CONNECT( "ctor start" );

        // wait for enrolement and registration to complete
        auto pFut     = m_pIOContext->post( boost::asio::use_future(
            [ this, ipAddress, port ]()
            { return m_pClient->connect( ipAddress, port ); } ) );
        m_pConnection = pFut.get();
        m_startupFuture.get();

        LOG_CONNECT( "Connection completed" );

        LogicalThread::registerFiber( m_enrolement.getMP() );

        // clang-format off
        auto creation = [ this ]
            ( Registry::Update::Ptr pUpdate ) 
        { 
            onCreate( pUpdate->registration );
            pUpdate->done();
        };

        writeRegistry()->setCreationCallback(
            [ this, creation = creation ]
            ( 
                Registry::Update::Ptr pUpdate 
            )
            {
                m_pIOContext->post
                ( 
                    [
                        creation, 
                        pUpdate
                    ]() mutable
                    {
                        boost::fibers::fiber(
                            [
                                creation, 
                                pUpdate
                            ]() mutable
                            {
                                creation( pUpdate );
                            }
                        ).detach();
                    }
                );
            } );
        // clang-format on

        LOG_CONNECT( "ctor end" );
    }

    ~Connect()
    {
        m_bShuttingDown = true;

        boost::fibers::promise< void > waitForServerShutdown;
        boost::fibers::future< void >  waitForServerShutdownFuture
            = waitForServerShutdown.get_future();

        m_pIOContext->post(
            [ &pClient = m_pClient, &waitForServerShutdown ]()
            {
                boost::fibers::fiber(
                    [ &pClient, &waitForServerShutdown ]
                    {
                        LOG_CLIENT( "shutdown fiber start" );
                        pClient->stop();
                        waitForServerShutdown.set_value();
                        LOG_CLIENT( "shutdown fiber stop" );
                    } )
                    .detach();
            } );

        waitForServerShutdownFuture.get();

        m_pIOContext->stop();

        LOG_CLIENT( "io service stopped" );

        m_thread.join();

        LOG_CLIENT( "dtor complete" );
    }

    MP getMP() const { return m_enrolement.getMP(); }
    MP getDaemonMP() const { return m_enrolement.getDaemon(); }

    void run() { LogicalThread::get().runMessageLoop(); }

private:
    void onCreate( const Registration& reg )
    {
        if( m_bShuttingDown )
        {
            LOG_CONNECT( "Ignoring onCreate because shutting down" );
            return;
        }
        LOG_CONNECT( "onCreate:\n" << reg );
        if( auto p = std::dynamic_pointer_cast< service::Connection >(
                m_pConnection.lock() ) )
        {
            sendRegistration( reg, { getMP() }, p );
        }
    }

    void connectCallback( service::Connection::Ptr pConnection )
    {
        if( m_bShuttingDown )
        {
            LOG_CONNECT(
                "Ignoring connectCallback because shutting down" );
            return;
        }
        LOG_CONNECT( "connect callback: "
                     << pConnection->getSocketInfo()
                     << " Process: " << pConnection->getProcessID() );
    }

    void disconnectCallback( service::Connection::Ptr pConnection )
    {
        if( m_bStarting )
        {
            m_bStarting = false;
            m_startupPromise.set_value();
        }
        LOG_CONNECT( "disconnect callback: "
                     << pConnection->getSocketInfo()
                     << " Process: " << pConnection->getProcessID() );
        m_bShuttingDown = true;
        LogicalThread::shutdownAll( getMP() );
    }

    void receiverCallback( Connection::WeakPtr pResponseConnection,
                           const PacketBuffer& buffer )
    {
        if( m_bShuttingDown )
        {
            LOG_CONNECT( "Ignoring message because shutting down" );
            return;
        }

        IArchive ia( *this, buffer );

        MessageType messageType;
        ia >> messageType;

        switch( messageType )
        {
            case MessageType::eEnrole:
            {
                ia >> m_enrolement;
                LOG_CONNECT( "Got enrolement from daemon: "
                             << m_enrolement.getDaemon()
                             << " with mp: "
                             << m_enrolement.getMP() );
            }
            break;
            case MessageType::eRegistry:
            {
                std::set< MP > mps;
                Registration   registration;
                ia >> mps;
                ia >> registration;
                LOG_CONNECT( "eRegistry:\n" << registration );

                // filter registration entries for THIS process
                // since ONLY created inter-process proxies
                registration.remove( m_enrolement.getMP() );
                writeRegistry()->update(
                    pResponseConnection, registration );
                if( m_bStarting )
                {
                    m_bStarting = false;
                    m_startupPromise.set_value();
                }
                LOG_CONNECT( "eRegistry complete" );
            }
            break;
            case MessageType::eDisconnect:
            {
                LOG_CONNECT( "eDisconnect" );
                std::set< MP > visited;
                MP             shutdownMP;

                ia >> visited;
                ia >> shutdownMP;

                writeRegistry()->disconnected( shutdownMP );
            }
            break;
            case MessageType::eRequest:
            {
                Header header;
                ia >> header;

                VERIFY_RTE_MSG( header.m_responder.getMP() == getMP(),
                                "Received request intended for "
                                "different responder: "
                                    << header << " when enrolement: "
                                    << m_enrolement );
                decodeInboundRequest(
                    *this, header, buffer, pResponseConnection );
            }
            break;
            case MessageType::eResponse:
            {
                Header header;
                ia >> header;

                LogicalThread::get( header.m_requester )
                    .send( InterProcessResponse{ header, buffer } );
            }
            break;
            case MessageType::TOTAL_MESSAGES:
            default:
            {
                THROW_RTE( " Unepxcted message type recieved" );
            }
            break;
        }
    }
};
} // namespace mega::service
