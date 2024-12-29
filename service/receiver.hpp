

#pragma once

#include "service/asio.hpp"
#include "service/sender.hpp"

#include "service/protocol/packet.hpp"

#include <array>
#include <iostream>

namespace mega::service
{
    template<typename Socket, typename DisconnectCallback >
    class Receiver
    {
    public:
        template< typename Functor >
        Receiver(Socket& socket, Functor disconnect_callback)
        :   m_socket(socket)
        ,   m_sender(m_socket)
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
                    }
                    else
                    {
                        // dispatch packet
                    }
                }

                m_disconnect_callback();

            }).detach();
        }

        void stop() { m_bContinue = false; }

        bool                m_bContinue = true;
        Socket&             m_socket; 
        SocketSender        m_sender;
        PacketBuffer        m_packetBuffer;
        DisconnectCallback  m_disconnect_callback;
    };
}

