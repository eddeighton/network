

#pragma once

#include "service/asio.hpp"
#include "service/sender.hpp"

#include <array>
#include <iostream>

namespace service
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
                    // read message
                    std::array<char, 4> buf;

                    boost::system::error_code ec;
                    auto szBytesTransferred
                        = boost::asio::async_read(
                            m_socket, 
                            boost::asio::buffer( buf ),
                            boost::fibers::asio::yield[ ec ] );

                    if( ec )
                    {
                        m_bContinue = false;
                    }
                    else
                    {
                        std::string s;
                        std::copy(buf.begin(), buf.end(),
                            std::back_inserter(s));
                        std::cout << "Received: " << s << std::endl;
                    }
                }

                m_disconnect_callback();

            }).detach();
        }

        void stop() { m_bContinue = false; }

        bool                m_bContinue = true;
        Socket&             m_socket; 
        SocketSender        m_sender;
        DisconnectCallback  m_disconnect_callback;
    };
}

