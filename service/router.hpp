
#pragma once

#include "service/protocol/header.hpp"
#include "service/connection.hpp"
#include "service/fibers.hpp"

#include "service/protocol/message.hpp"
#include "service/protocol/stack.hpp"

#include "vocab/service/mp.hpp"

#include "common/log.hpp"
#include "common/time.hpp"

#include <unordered_map>
#include <variant>
#include <map>
#include <memory>

using namespace std::string_literals;
#define LOG_ROUTER( msg ) LOG( "ROUTER: "s + msg )
// #define LOG_ROUTER(msg)

namespace mega::service
{
class Router
{
public:
    struct ServiceMessage
    {
        MessageType         m_messageType;
        Header              m_header;
        PacketBuffer        m_buffer;
        Connection::WeakPtr m_pResponseConnection;
    };
    struct Connect
    {
        MP              mp;
        Connection::Ptr pConnection;
    };
    struct Disconnect
    {
        MP              mp;
        Connection::Ptr pConnection;
    };
    struct Shutdown
    {
    };
    using Msg = std::variant< std::monostate,
                              ServiceMessage,
                              Connect,
                              Disconnect,
                              Shutdown >;

private:
    using Channel = boost::fibers::buffered_channel< Msg >;

public:
    using DirectConnections
        = std::unordered_map< MP, Connection::WeakPtr, MP::Hash >;
    using IndirectConnections
        = std::unordered_map< MP, Connection::WeakPtr, MP::Hash >;
    using Ptr = std::unique_ptr< Router >;
    using Map = std::map< Stack, Ptr >;

    class Table
    {
        // direct connections are maintained by daemon and always
        // correct
        DirectConnections m_direct;
        // indirect connections are an estimate and may be incorrect
        IndirectConnections m_indirect;

    public:
        inline const DirectConnections& getDirect() const
        {
            return m_direct;
        }
        inline void addDirect( MP mp, Connection::Ptr pConnection )
        {
            m_direct.insert( { mp, pConnection } );
        }
        inline void removeDirect( MP mp )
        {
            m_direct.erase( mp );
        }
        inline IndirectConnections& getIndirect()
        {
            return m_indirect;
        }
    };

private:
    inline Msg receive()
    {
        Msg  message;
        auto status = m_channel.pop( message );
        if( status != boost::fibers::channel_op_status::success )
        {
            THROW_RTE( "Failed to dispatch message" );
        }
        return message;
    }

public:
    inline Router( Stack stack, Table& table )
        : m_stack( std::move( stack ) )
        , m_table( table )
        , m_channel( 16 )
        , m_fiber(
              [ this ]()
              {
                  LOG_ROUTER( "start" );
                  run();
                  LOG_ROUTER( "stop" );
              } )
    {
    }

    ~Router()
    {
        m_bContinue = false;
        send( Shutdown{} );
        m_fiber.join();
    }

    inline void run()
    {
        while( m_bContinue )
        {
            const auto msg = receive();

            if( const ServiceMessage* pMsg
                = std::get_if< ServiceMessage >( &msg ) )
            {
                auto startTime = std::chrono::steady_clock::now();

                LOG_ROUTER(
                    "Got Service Message: " << pMsg->m_header );
                VERIFY_RTE_MSG(
                    pMsg->m_messageType == MessageType::eRequest,
                    "Expected request in router for msg: "
                        << pMsg->m_header );
                // attempt to find direct connection to route message
                Connection::WeakPtr pOriginalRequestResponseConnection
                    = pMsg->m_pResponseConnection;
                bool bDirectSend = false;
                auto direct      = m_table.getDirect();
                auto iFind       = direct.find(
                    pMsg->m_header.m_responder.getMP() );
                if( iFind != direct.end() )
                {
                    // route the message using direct connection which
                    // should always be correct
                    if( auto pCon = iFind->second.lock() )
                    {
                        LOG_ROUTER( "Sending request direct: "
                                    << pMsg->m_header );
                        pCon->send( pMsg->m_buffer );
                        // now wait for response
                        auto response = receive();

                        bool bDirectResponse = false;
                        if( const ServiceMessage* pResponseMsg
                            = std::get_if< ServiceMessage >(
                                &response ) )
                        {
                            if( auto pConResponse
                                = pOriginalRequestResponseConnection
                                      .lock() )
                            {
                                LOG_ROUTER(
                                    "Sending response direct: "
                                    << pMsg->m_header );
                                pConResponse->send(
                                    pResponseMsg->m_buffer );
                                bDirectResponse = true;
                            }
                            else
                            {
                                //  ?
                            }
                        }
                        else
                        {
                            THROW_TODO;
                        }
                        bDirectSend = true;
                    }
                    else
                    {
                        THROW_RTE( "Lost connection" );
                    }
                }
                if( !bDirectSend )
                {
                    // no direct connection to attempt indirect
                    // routing
                    THROW_TODO;
                }

                LOG_ROUTER( "Dispatch of: "
                            << pMsg->m_header << " in : "
                            << common::printDuration(
                                   common::elapsed( startTime ) ) );
            }
            else
            {
            }
        }
    }

    inline void send( Msg msg )
    {
        auto status = m_channel.push( std::move( msg ) );
        VERIFY_RTE_MSG(
            status == boost::fibers::channel_op_status::success,
            "Error sending message to channel" );
    }

private:
    bool                 m_bContinue{ true };
    Stack                m_stack;
    Table&               m_table;
    Channel              m_channel;
    boost::fibers::fiber m_fiber;
};
} // namespace mega::service
