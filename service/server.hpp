

#pragma once

#include "service/port_number.hpp"
#include "service/asio.hpp"
#include "service/receiver.hpp"
#include "service/sender_socket.hpp"
#include "service/socket_info.hpp"
#include "service/connection.hpp"
#include "service/acceptor.hpp"

#include "common/log.hpp"

#include <boost/circular_buffer.hpp>

// using namespace std::string_literals;
// #define LOG_SERVER(msg) LOG("SERVER: "s + msg)
#define LOG_SERVER( msg )

namespace mega::service
{
class Server
{
public:
    class Connection : public service::Connection
    {
        friend class Server;
        friend class Acceptor< std::shared_ptr< Connection > >;

        using DisconnectCallback = std::function< void() >;
        using Strand             = boost::asio::strand<
                        boost::asio::io_context::executor_type >;
        using Socket            = boost::asio::ip::tcp::socket;
        using SocketReceiver    = Receiver< Socket >;
        using SocketReceiverPtr = std::unique_ptr< SocketReceiver >;

    public:
        using Ptr = std::shared_ptr< Connection >;

        Connection( Server& server )
            : m_server( server )
            , m_strand(
                  boost::asio::make_strand( m_server.m_ioContext ) )
            , m_socket( m_strand )
            , m_sender( m_socket )
            , m_processID( PROCESS_ZERO )
        {
        }

    private:
        void start( ProcessID processID )
        {
            LOG_SERVER( "connection start " << m_socket_info );
            m_processID   = processID;
            m_socket_info = TCPSocketInfo::make( m_socket );
            VERIFY_RTE( !m_pReceiver );
            m_pReceiver = std::make_unique< SocketReceiver >(
                shared_from_this(),
                m_socket,
                m_server.m_receiverCallback,
                [ &server = m_server ](
                    service::Connection::WeakPtr pConnection )
                { server.onDisconnect( pConnection ); } );
        }

    public:
        const TCPSocketInfo& getSocketInfo() const override
        {
            return m_socket_info;
        }

        ProcessID getProcessID() const override
        {
            return m_processID;
        }

        void stop() override
        {
            m_pReceiver.reset();
            LOG_SERVER( "connection stoppping " << m_socket_info );
        }

        void send( const PacketBuffer& buffer ) override
        {
            if( m_socket.is_open() )
            {
                auto ec = m_sender.send( buffer );
                VERIFY_RTE_MSG( !ec,
                    "Error attempting to send buffer: " << ec.what() );
            }
            else
            {
                THROW_RTE( "Cannot send of disconnected connection" );
            }
        }

    private:
        Server&           m_server;
        Strand            m_strand;
        Socket            m_socket;
        TCPSocketInfo     m_socket_info;
        SocketSender      m_sender;
        ProcessID         m_processID;
        SocketReceiverPtr m_pReceiver;
    };

private:
    using Acceptor          = service::Acceptor< Connection::Ptr >;
    using AcceptorPtr       = Acceptor::Ptr;
    using ProcessIDFreeList = boost::circular_buffer< ProcessID >;
    using ConnectionPtrMap  = std::map< ProcessID, Connection::Ptr >;

public:
    using ConnectionCallback
        = std::function< void( service::Connection::Ptr ) >;
    using DisconnectCallback
        = std::function< void( service::Connection::Ptr ) >;

    Server( boost::asio::io_context& ioContext,
            PortNumber               port_number,
            ReceiverCallback         receiverCallback,
            ConnectionCallback       connectionCallback,
            DisconnectCallback       disconnectCallback )
        : m_ioContext( ioContext )
        , m_port_number( port_number )
        , m_receiverCallback( std::move( receiverCallback ) )
        , m_connectionCallback( std::move( connectionCallback ) )
        , m_disconnectCallback( std::move( disconnectCallback ) )
        , m_processIDFreeList(
              std::numeric_limits< ProcessID::ValueType >::max() - 1 )
    {
        // simply add ALL valid ProcessIDs to the freelist on startup
        for( int i = 1;
             i != std::numeric_limits< ProcessID::ValueType >::max();
             ++i )
        {
            m_processIDFreeList.push_back( ProcessID{
                static_cast< ProcessID::ValueType >( i ) } );
        }
    }

    ~Server() { stop(); }

    void start()
    {
        m_pAcceptor = std::make_unique< Acceptor >(
            m_ioContext,
            boost::asio::ip::tcp::endpoint(
                boost::asio::ip::tcp::v4(), m_port_number.value ),
            [ this ]() -> Connection::Ptr
            { return std::make_shared< Connection >( *this ); },
            [ this ]( Connection::Ptr pConnection )
            { onConnect( pConnection ); } );
    }

    void stop()
    {
        LOG_SERVER( "stop" );
        m_pAcceptor.reset();
        LOG_SERVER( "stop acceptor joined" );
        auto con = m_connections;
        for( auto& c : con )
        {
            c.second->stop();
        }
        LOG_SERVER( "stop complete" );
    }

private:
    void onConnect( Connection::Ptr pNewConnection )
    {
        VERIFY_RTE_MSG( !m_processIDFreeList.empty(),
                        "No free ProcessIDs available" );

        const auto processID = m_processIDFreeList.front();
        m_processIDFreeList.pop_front();
        m_connections.insert(
            std::make_pair( processID, pNewConnection ) );
        pNewConnection->start( processID );
        m_connectionCallback( pNewConnection );
    }

    void
    onDisconnect( service::Connection::WeakPtr pWeakConnectionPtr )
    {
        LOG_SERVER( "onDisconnect" );
        if( auto p = pWeakConnectionPtr.lock() )
        {
            Connection::Ptr pConnection
                = std::dynamic_pointer_cast< Connection >( p );
            VERIFY_RTE( pConnection );
            auto processID = pConnection->getProcessID();
            auto iFind     = m_connections.find( processID );
            VERIFY_RTE_MSG( iFind != m_connections.end(),
                            "Invalid call to onDisconnect for "
                            "already disconnected connection" );

            m_connections.erase( iFind );
            m_processIDFreeList.push_back( processID );
            LOG_SERVER( "onDisconnect calling callback" );
            m_disconnectCallback( pConnection );
        }
        LOG_SERVER( "onDisconnect complete" );
    }

    boost::asio::io_context& m_ioContext;
    PortNumber               m_port_number;
    ReceiverCallback         m_receiverCallback;
    ConnectionCallback       m_connectionCallback;
    DisconnectCallback       m_disconnectCallback;
    ConnectionPtrMap         m_connections;
    ProcessIDFreeList        m_processIDFreeList;
    AcceptorPtr              m_pAcceptor;
};
} // namespace mega::service
