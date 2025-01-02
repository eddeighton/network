
#pragma once

#include "service/client.hpp"
#include "service/registry.hpp"
#include "service/network.hpp"
#include "service/enrole.hpp"

#include "service/protocol/message.hpp"
#include "service/protocol/serialization.hpp"

#include <boost/fiber/operations.hpp>

#include <filesystem>
#include <vector>
#include <string>
#include <iostream>
#include <chrono>
#include <functional>

namespace mega::service
{
    class Connect
    {
        Network                         m_network;
        Enrole                          m_enrolement;
        boost::fibers::promise<void>    m_waitForRegistryPromise;
        Client                          m_client;
        Client::Connection::Ptr         m_pConnection;
    public:
        Connect(IPAddress ipAddress, PortNumber port)
            :  m_client( m_network, std::bind( &Connect::receiverCallback,
                            this, std::placeholders::_1, std::placeholders::_2 ) )
        {
            boost::fibers::future<void> registrationFuture =
                m_waitForRegistryPromise.get_future();

            m_pConnection = m_client.connect(ipAddress, port);

            // wait for enrolement and registration to complete
            registrationFuture.get();

            mega::service::LogicalThread::registerFiber(m_enrolement.getMP());
        }

        ~Connect()
        {
            m_pConnection->stop();
        }

        Network& getNetwork() { return m_network; }

        void run()
        {
            mega::service::LogicalThread::get().runMessageLoop();
        }

        void receiverCallback(
                mega::service::SocketSender& responseSender,
                const mega::service::PacketBuffer& buffer)
        {
            
            boost::interprocess::basic_vectorbuf
                < mega::service::PacketBuffer > vectorBuffer(buffer);
            boost::archive::binary_iarchive ia(vectorBuffer, 
                    mega::service::boostArchiveFlags);

            mega::service::MessageType messageType;
            ia >> messageType;

            switch( messageType )
            {
                case mega::service::MessageType::eEnrole:
                    {                                   
                        ia >> m_enrolement;
                        std::cout << "Got enrolement from daemon: " 
                            << m_enrolement.getDaemon() << " with mp: "
                            << m_enrolement.getMP() << std::endl; 
                    }
                    break;
                case mega::service::MessageType::eRegistry:
                    {
                        std::cout << "Got registration update" << std::endl;
                        mega::service::Registration registration;
                        ia >> registration;
                        // filter registration entries for THIS process
                        // since ONLY created inter-process proxies
                        registration.filter(m_enrolement.getMP());
                        mega::service::Registry::getWriteAccess()->update(
                            responseSender, registration);
                        m_waitForRegistryPromise.set_value();
                    }
                    break;
                case mega::service::MessageType::eRequest:
                    {
                        mega::service::decodeInboundRequest(ia, responseSender);
                    }
                    break;
                case mega::service::MessageType::eResponse:
                    {
                        mega::service::Header header;
                        ia >> header;

                        mega::service::LogicalThread& logicalThread =
                            [&]() -> mega::service::LogicalThread&
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

