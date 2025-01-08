

#pragma once

#include "service/gen/decoder.hxx"

#include "service/asio.hpp"
#include "service/connection.hpp"

#include "service/protocol/header.hpp"
#include "service/protocol/packet.hpp"

#include "common/log.hpp"

#include <array>
#include <iostream>
#include <functional>

// using namespace std::string_literals;
// #define LOG_RECEIVER(msg) LOG("RECEIVER: "s + msg)
#define LOG_RECEIVER(msg)

namespace mega::service
{
    using ReceiverCallback = std::function< void( Connection::WeakPtr, const PacketBuffer& ) >;
    using DisconnectCallback = std::function< void( Connection::WeakPtr ) >;
    
    template<typename Socket >
    class Receiver
    {
    public:
        Receiver(Connection::WeakPtr pConnection, 
                Socket& socket, 
                ReceiverCallback receiverCallback, 
                DisconnectCallback disconnect_callback)
        :   m_pConnection(pConnection)
        ,   m_socket(socket)
        ,   m_callback( std::move(receiverCallback) )
        ,   m_disconnect_callback( std::move( disconnect_callback ) )
        ,   m_fiber( [this](){ run(); } )
        {
            LOG_RECEIVER( "RECEIVER: ctor" ) ;
        }

        ~Receiver()
        {
            stop();
            m_fiber.join();
            LOG_RECEIVER( "RECEIVER: dtor" ) ;
        }

    private:
        void run()
        {
            LOG_RECEIVER( "RECEIVER: fiber started" ) ;
            while( m_bContinue && m_socket.is_open() )
            {
                const auto error = receive( m_socket, m_packetBuffer );

                //LOG_RECEIVER( "Received packet with error: " << error.what() ) ;

                if( error.value() == boost::asio::error::eof )
                {
                    //  This is what happens when close socket normally
                    m_bContinue = false;
                    //LOG_RECEIVER( "Socket returned eof" ) ;
                }
                else if( error.value() == boost::asio::error::operation_aborted )
                {
                    //  This is what happens when close socket normally
                    m_bContinue = false;
                    //LOG_RECEIVER( "Socket returned operation aborted" ) ;
                }
                else if( error.value() == boost::asio::error::connection_reset )
                {
                    m_bContinue = false;
                    //LOG_RECEIVER( "Socket returned connection reset" ) ;
                }
                else if( error.failed() )
                {
                    m_bContinue = false;
                    //LOG_RECEIVER( "Critical socket failure: " << error.what() ) ;
                    // std::abort();
                }
                else
                {
                    m_callback(m_pConnection, m_packetBuffer);
                }
            }
            boost::fibers::fiber( 
                [
                    pConnection = m_pConnection, 
                    callback = std::move(m_disconnect_callback)
                ]()
                {
                    LOG_RECEIVER( "RECEIVER: shutdown callback" ) ;
                    callback(pConnection);
                }).detach();
            LOG_RECEIVER( "RECEIVER: fiber shutdown" ) ;
        }

    public:
        void stop()
        { 
            if( m_socket.is_open() )
            {
                boost::system::error_code ec;
                m_socket.shutdown( m_socket.shutdown_both, ec );
                m_socket.close( ec );
            }
            m_bContinue = false; 
        }

    private:
        bool                m_bContinue = true;
        Connection::WeakPtr m_pConnection;
        Socket&             m_socket; 
        PacketBuffer        m_packetBuffer;
        ReceiverCallback    m_callback;
        DisconnectCallback  m_disconnect_callback;
        boost::fibers::fiber m_fiber;
    };
}

