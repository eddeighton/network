

#pragma once

#include "service/gen/decoder.hxx"

#include "service/asio.hpp"
#include "service/connection.hpp"

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
    using ReceiverCallback = std::function< void( Connection::WeakPtr, const PacketBuffer& ) >;
    
    template<typename Socket, typename DisconnectCallback >
    class Receiver
    {
    public:
        template< typename Functor >
        Receiver(Socket& socket, ReceiverCallback receiverCallback, Functor disconnect_callback)
        :   m_socket(socket)
        ,   m_callback( std::move(receiverCallback) )
        ,   m_disconnect_callback( std::move( disconnect_callback ) )
        //,   m_waitForServerShutdownFuture(m_waitForServerShutdown.get_future())
        {
        }

        ~Receiver()
        {
            // m_waitForServerShutdownFuture.get();
        }

        void run(Connection::WeakPtr pConnection)
        {
            m_bStarted = true;
            boost::fibers::fiber( [this, pConnection]
            {
                //std::cout << "Receiver started" << std::endl;
                while( m_bContinue && m_socket.is_open() )
                {
                    const auto error = receive( m_socket, m_packetBuffer );

                    //std::cout << "Received packet with error: " << error.what() << std::endl;

                    if( error.value() == boost::asio::error::eof )
                    {
                        //  This is what happens when close socket normally
                        m_bContinue = false;
                        //std::cout << "Socket returned eof" << std::endl;
                    }
                    else if( error.value() == boost::asio::error::operation_aborted )
                    {
                        //  This is what happens when close socket normally
                        m_bContinue = false;
                        //std::cout << "Socket returned operation aborted" << std::endl;
                    }
                    else if( error.value() == boost::asio::error::connection_reset )
                    {
                        m_bContinue = false;
                        //std::cout << "Socket returned connection reset" << std::endl;
                    }
                    else if( error.failed() )
                    {
                        m_bContinue = false;
                        //std::cout << "Critical socket failure: " << error.what() << std::endl;
                        // std::abort();
                    }
                    else
                    {
                        m_callback(pConnection, m_packetBuffer);
                    }
                }
                m_disconnect_callback();
                //m_waitForServerShutdown.set_value();
                std::cout << "Receiver shutdown" << std::endl;
            }).detach();
        }

        void stop() { m_bContinue = false; }
        bool started() const { return m_bStarted; }

        bool                m_bContinue = true;
        bool                m_bStarted = false;
        Socket&             m_socket; 
        PacketBuffer        m_packetBuffer;
        ReceiverCallback    m_callback;
        DisconnectCallback  m_disconnect_callback;
        // boost::fibers::promise<void>    m_waitForServerShutdown;
        // boost::fibers::future<void>     m_waitForServerShutdownFuture;
    };
}

