
#pragma once

#include "service/client.hpp"
#include "service/access.hpp"
#include "service/registry.hpp"
#include "service/network.hpp"
#include "service/enrole.hpp"

#include "service/gen/decoder.hxx"

#include "service/protocol/message.hpp"
#include "service/protocol/serialization.hpp"
#include "service/protocol/message_factory.hpp"

#include "common/log.hpp"

#include <boost/fiber/operations.hpp>

#include <filesystem>
#include <vector>
#include <string>
#include <iostream>
#include <chrono>
#include <functional>
#include <optional>

// disable logging
// #define LOG_CONNECT(msg)
using namespace std::string_literals;
#define LOG_CONNECT(msg) LOG( "CONNECT: "s + msg )

namespace mega::service
{
    class Connect : public Access
    {
        Network                         m_network;
        Enrole                          m_enrolement;
        std::optional< boost::fibers::promise<void> > m_waitForRegistryPromise;
        Client                          m_client;
        Client::Connection::Ptr         m_pConnection;
    public:
        Connect(IPAddress ipAddress, PortNumber port)
            :  m_client
               (
                    m_network.getIOContext(),
                    std::bind( &Connect::receiverCallback,
                        this, std::placeholders::_1, std::placeholders::_2 ),
                    [this]( Connection::Ptr pConnection ){ connectCallback( pConnection ); },
                    [this]( Connection::Ptr pConnection ){ disconnectCallback( pConnection ); }
               )
        {
            LOG_CONNECT( "ctor start" ) ;
            m_waitForRegistryPromise = boost::fibers::promise<void>{};
            auto registrationFuture = m_waitForRegistryPromise->get_future();

            m_pConnection = m_client.connect(ipAddress, port);

            // wait for enrolement and registration to complete
            LOG_CONNECT( "waiting for future" ) ;
            registrationFuture.get();
            LOG_CONNECT( "ctor got future" ) ;
            m_waitForRegistryPromise.reset();

            LogicalThread::registerFiber(m_enrolement.getMP());

            Connection::WeakPtr pWeak = m_pConnection;
            // setup registry creation callback
            writeRegistry()->setCreationCallback(
                [pWeak, mp = getMP()](Registration reg)
                {
                    if( auto pCon = pWeak.lock() )
                    {
                        sendRegistration( reg, { mp }, pCon );
                    }
                    else
                    {
                        THROW_RTE( "Cannot use creation callback connection" );
                    }
                });
            LOG_CONNECT( "ctor end" ) ;
        }

        ~Connect()
        {
            m_pConnection->stop();
        }

        Network& getNetwork() { return m_network; }
        MP getMP() const { return m_enrolement.getMP(); }
        MP getDaemonMP() const { return m_enrolement.getDaemon(); }

        void run()
        {
            LogicalThread::get().runMessageLoop();
        }

    private:
        void connectCallback(service::Connection::Ptr pConnection)
        {
            LOG_CONNECT( "connect callback: " << pConnection->getSocketInfo() <<
                 " Process: " << pConnection->getProcessID() );
        }

        void disconnectCallback(service::Connection::Ptr pConnection)
        {
            LOG_CONNECT( "disconnect callback: " << pConnection->getSocketInfo() <<
                " Process: " << pConnection->getProcessID() );
            if( m_waitForRegistryPromise.has_value() )
            {
                m_waitForRegistryPromise->set_value();
            }
            LogicalThread::shutdownAll();
        }

        void receiverCallback(
                Connection::WeakPtr pResponseConnection,
                const PacketBuffer& buffer)
        {
            IArchive ia(*this, buffer);

            MessageType messageType;
            ia >> messageType;

            switch( messageType )
            {
                case MessageType::eEnrole:
                    {                                   
                        ia >> m_enrolement;
                        LOG_CONNECT( "Got enrolement from daemon: " 
                           << m_enrolement.getDaemon() << " with mp: "
                           << m_enrolement.getMP() ) ; 
                    }
                    break;
                case MessageType::eRegistry:
                    {
                        LOG_CONNECT( "eRegistry");
                        std::vector< MP > mps;
                        Registration registration;
                        ia >> mps;
                        ia >> registration;
                        // filter registration entries for THIS process
                        // since ONLY created inter-process proxies
                        registration.remove(m_enrolement.getMP());
                        writeRegistry()->update(pResponseConnection, registration);
                        if( m_waitForRegistryPromise.has_value() )
                        {
                            m_waitForRegistryPromise->set_value();
                        }
                    }
                    break;
                case MessageType::eDisconnect      :
                    {
                        LOG_CONNECT( "eDisconnect" );
                        std::set< MP > visited;
                        MP shutdownMP;

                        ia >> visited;
                        ia >> shutdownMP;

                        writeRegistry()->disconnected(shutdownMP);
                        if( m_waitForRegistryPromise.has_value() )
                        {
                            m_waitForRegistryPromise->set_value();
                        }
                    }
                    break;
                case MessageType::eRequest:
                    {
                        Header header;
                        ia >> header;

                        VERIFY_RTE_MSG(header.m_responder.getMP() == getMP(),
                            "Received request intended for different responder: " << header <<
                            " when enrolement: " << m_enrolement );
                        decodeInboundRequest(*this, header, buffer, pResponseConnection);
                    }
                    break;
                case MessageType::eResponse:
                    {
                        Header header;
                        ia >> header;

                        LogicalThread::get(header.m_requester).send(
                            InterProcessResponse
                            {
                                header,
                                buffer
                            }
                        );
                    }
                    break;
                case MessageType::TOTAL_MESSAGES:
                default:
                    {
                        THROW_RTE(" Unepxcted message type recieved" );
                    }
                    break;
            }
        }
    };
}

