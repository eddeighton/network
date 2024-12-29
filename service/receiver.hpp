

#pragma once

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
                        
                        static constexpr auto boostArchiveFlags = boost::archive::no_header | boost::archive::no_codecvt
                                          | boost::archive::no_xml_tag_checking | boost::archive::no_tracking;
                        boost::archive::binary_iarchive ia( m_vectorBuffer, boostArchiveFlags );

                        Header header;
                        ia >> header;

                        // determine the target InProcessProxy and logical thread
                        

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
        boost::interprocess::basic_vectorbuf< service::PacketBuffer > m_vectorBuffer;
        DisconnectCallback  m_disconnect_callback;
    };
}

