
#pragma once

#include "service/client.hpp"
#include "service/registry.hpp"
#include "service/network.hpp"
#include "service/enrole.hpp"

#include "service/gen/decoder.hxx"

#include "service/protocol/message.hpp"
#include "service/protocol/serialization.hpp"
#include "service/protocol/message_factory.hpp"

#include <boost/fiber/operations.hpp>

#include <filesystem>
#include <vector>
#include <string>
#include <iostream>
#include <chrono>
#include <functional>
#include <optional>

namespace mega::service
{
    class Connect
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
                    std::bind( &Connect::receiverCallback, this, std::placeholders::_1, std::placeholders::_2 ),
                    [this]( Connection::Ptr pConnection ){},
                    [this]( Connection::Ptr pConnection ){}
               )
        {
            m_waitForRegistryPromise = boost::fibers::promise<void>{};
            auto registrationFuture = m_waitForRegistryPromise->get_future();

            m_pConnection = m_client.connect(ipAddress, port);

            // wait for enrolement and registration to complete
            registrationFuture.get();
            m_waitForRegistryPromise.reset();

            mega::service::LogicalThread::registerFiber(m_enrolement.getMP());

            Connection::WeakPtr pWeak = m_pConnection;
            // setup registry creation callback
            mega::service::Registry::getWriteAccess()->setCreationCallback(
                [pWeak, mp = getMP()](Registration reg)
                {
                    std::cout << "Generated reg update of: " << reg << std::endl;
                    if( auto pCon = pWeak.lock() )
                    {
                        sendRegistration( reg, { mp }, pCon->getSender() );
                    }
                    else
                    {
                        THROW_RTE( "Cannot use creation callback connection" );
                    }
                });
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
            mega::service::LogicalThread::get().runMessageLoop();
        }

        void receiverCallback(
                Connection::WeakPtr pResponseConnection,
                const mega::service::PacketBuffer& buffer)
        {
            boost::interprocess::basic_vectorbuf< mega::service::PacketBuffer > vectorBuffer(buffer);
            boost::archive::binary_iarchive ia(vectorBuffer, mega::service::boostArchiveFlags);

            mega::service::MessageType messageType;
            ia >> messageType;

            switch( messageType )
            {
                case mega::service::MessageType::eEnrole:
                    {                                   
                        ia >> m_enrolement;
                        // std::cout << "Got enrolement from daemon: " 
                        //   << m_enrolement.getDaemon() << " with mp: "
                        //   << m_enrolement.getMP() << std::endl; 
                    }
                    break;
                case mega::service::MessageType::eRegistry:
                    {
                        std::vector< MP > mps;
                        mega::service::Registration registration;
                        ia >> mps;
                        ia >> registration;
                        // filter registration entries for THIS process
                        // since ONLY created inter-process proxies
                        registration.filter(m_enrolement.getMP());
                        mega::service::Registry::getWriteAccess()->update(
                            pResponseConnection.lock()->getSender(), registration);
                        std::cout << "Got registration update: " << registration << std::endl;
                        if( m_waitForRegistryPromise.has_value() )
                        {
                            m_waitForRegistryPromise->set_value();
                        }
                    }
                    break;
                case mega::service::MessageType::eRequest:
                    {
                        mega::service::Header header;
                        ia >> header;

                        VERIFY_RTE_MSG(header.m_responder.getMP() == getMP(),
                            "Received request intended for different responder: " << header <<
                            " when enrolement: " << m_enrolement );
                        mega::service::decodeInboundRequest(ia, header, pResponseConnection);
                    }
                    break;
                case mega::service::MessageType::eResponse:
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
                case mega::service::MessageType::TOTAL_MESSAGES:
                default:
                    {
                        THROW_RTE(" Unepxcted message type recieved" );
                    }
                    break;
            }
        }
    };
}

