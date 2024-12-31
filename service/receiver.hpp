

#pragma once

#include "service/gen/decoder.hxx"

#include "service/asio.hpp"
#include "service/sender_socket.hpp"

#include "service/protocol/header.hpp"
#include "service/protocol/packet.hpp"

#include <boost/interprocess/interprocess_fwd.hpp>
#include <boost/interprocess/streams/vectorstream.hpp>

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>

#include <array>
#include <iostream>
#include <functional>

namespace mega::service
{
    using ReceiverCallback = std::function< void( SocketSender& responseSender, const PacketBuffer& ) >;
    
    template<typename Socket, typename DisconnectCallback >
    class Receiver
    {
    public:
        template< typename Functor >
        Receiver(Socket& socket, ReceiverCallback receiverCallback, Functor disconnect_callback)
        :   m_socket(socket)
        ,   m_sender(m_socket)
        ,   m_callback( std::move(receiverCallback) )
        ,   m_disconnect_callback( std::move( disconnect_callback ) )
        {
        }

        void run()
        {
            boost::fibers::fiber( [this]
            {
                while( m_bContinue && m_socket.is_open() )
                {
                    auto error = receive( m_socket, m_packetBuffer );
                    if( error.failed() )
                    {
                        onSocketError( error );
                        m_bContinue = false;
                    }
                    else
                    {
                        m_callback(m_sender, m_packetBuffer);
                    }
                }
                m_disconnect_callback();
                std::cout << "Receiver shutdown" << std::endl;
            }).detach();
        }

        void stop() { m_bContinue = false; }

        bool                m_bContinue = true;
        Socket&             m_socket; 
        SocketSender        m_sender;
        PacketBuffer        m_packetBuffer;
        ReceiverCallback    m_callback;
        DisconnectCallback  m_disconnect_callback;
    };
}

