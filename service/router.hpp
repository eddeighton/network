
#pragma once

#include "service/protocol/header.hpp"
#include "service/connection.hpp"
#include "service/fibers.hpp"

#include "service/protocol/message.hpp"
#include "service/protocol/stack.hpp"

#include "vocab/service/mp.hpp"

#include <vector>
#include <unordered_map>
#include <variant>
#include <map>
#include <memory>

namespace mega::service
{
    class Router
    {
    public:
        struct ServiceMessage
        {
            MessageType m_messageType;
            Header m_header;
            PacketBuffer m_buffer;
            Connection::WeakPtr m_pResponseConnection;
        };
        struct Connect
        {
            MP mp;
            Connection::Ptr pConnection;
        };
        struct Disconnect
        {
            MP mp;
            Connection::Ptr pConnection;
        };
        using Msg = std::variant< std::monostate, ServiceMessage, Connect, Disconnect >;
    private:
        using Channel = boost::fibers::buffered_channel< Msg >;
    public:
        using DirectConnections = std::unordered_map< MP, Connection::WeakPtr, MP::Hash >;
        using IndirectConnections = std::unordered_map< MP, Connection::WeakPtr, MP::Hash >;
        using Ptr = std::unique_ptr< Router >;
        using Map = std::map< Stack, Ptr >;

        struct Table
        {
            // direct connections are maintained by daemon and always correct
            DirectConnections direct;
            // indirect connections are an estimate and may be incorrect
            IndirectConnections indirect;
        };
    private:
        inline Msg receive()
        {
            Msg message;
            auto status = m_channel.pop(message);
            if( status != boost::fibers::channel_op_status::success )
            {
                THROW_RTE("Failed to dispatch message");
            }
            return message;
        }
    public:
        inline Router( Stack stack, Table& table )
        :   m_stack( std::move( stack ) )
        ,   m_table( table )
        ,   m_channel( 16 )
        {
            boost::fibers::fiber(
                [this]()
                {
                   run(); 
                }).detach();
        }

        inline void run()
        {
            while( m_bContinue )
            {
                const auto msg = receive();
             
                if( const ServiceMessage* pMsg = std::get_if< ServiceMessage >( &msg ) )
                {
                    VERIFY_RTE_MSG( pMsg->m_messageType == MessageType::eRequest,
                            "Expected request in router for msg: " << pMsg->m_header );
                    // attempt to find direct connection to route message
                    Connection::WeakPtr pOriginalRequestResponseConnection
                        = pMsg->m_pResponseConnection;
                    bool bDirectSend = false;
                    auto iFind =
                        m_table.direct.find( pMsg->m_header.m_responder.getMP() );
                    if( iFind != m_table.direct.end() )
                    {
                        // route the message using direct connection which should always be correct
                        if( auto pCon = iFind->second.lock() )
                        {
                            pCon->getSender().send( pMsg->m_buffer );
                            // now wait for response
                            auto response = receive();

                            bool bDirectResponse = false;
                            if( const ServiceMessage* pResponseMsg =
                                std::get_if< ServiceMessage >( &response ) )
                            {
                                if( auto pConResponse = pOriginalRequestResponseConnection.lock() )
                                {
                                    pConResponse->getSender().send( pResponseMsg->m_buffer );
                                    bDirectResponse = true;
                                }
                            }
                            else
                            {
                                THROW_TODO;
                            }
                            bDirectSend = true;
                        }
                    }
                    if( !bDirectSend )
                    {
                        // no direct connection to attempt indirect routing
                        THROW_TODO;
                    }
                }
                else
                {
                    THROW_TODO;
                }
            }
        }
    
        inline void send( Msg msg )
        {
            auto status = m_channel.push( std::move( msg ) );
            VERIFY_RTE_MSG(status == boost::fibers::channel_op_status::success,
                "Error sending message to channel" );
        }

    private:
        bool m_bContinue{ true };
        Stack m_stack;
        Table& m_table;
        Channel m_channel;
    };
}

