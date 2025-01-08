
#pragma once

#include "service/port_number.hpp"
#include "service/ip_address.hpp"
#include "service/asio.hpp"
#include "service/receiver.hpp"
#include "service/sender_socket.hpp"
#include "service/socket_info.hpp"
#include "service/network.hpp"
#include "service/connection.hpp"

#include "common/log.hpp"

#include <set>
#include <iostream>

// using namespace std::string_literals;
// #define LOG_CLIENT(msg) LOG("CLIENT: "s + msg)
#define LOG_CLIENT(msg)

namespace mega::service
{
    class Client
    {
    public:        
        class Connection : public service::Connection
        {
            friend class Client;

            using DisconnectCallback  = std::function< void() >;
            using Strand              = boost::asio::strand< 
                boost::asio::io_context::executor_type >;
            using Socket              = boost::asio::ip::tcp::socket;
            using Resolver            = boost::asio::ip::tcp::resolver;
            using EndPoint            = boost::asio::ip::tcp::endpoint;
            using SocketReceiver      = Receiver< Socket >;
            using SocketReceiverPtr   = std::unique_ptr< SocketReceiver >;

        public:
            using Ptr = std::shared_ptr< Connection >;

            Connection(Client& client, IPAddress ip_address, PortNumber port_number)
            :   m_client(client)
            ,   m_ip_address(std::move(ip_address))
            ,   m_port_number(std::move(port_number))
            ,   m_resolver(client.m_io_context)
            ,   m_strand( boost::asio::make_strand( client.m_io_context ) )
            ,   m_socket( m_strand )
            ,   m_sender( m_socket )
            {
                LOG_CLIENT( "Connection ctor start" ) ;
                Resolver::query        query( m_ip_address.value, m_port_number.str() );
                Resolver::results_type endpoints = m_resolver.resolve( query );

                if( endpoints.empty() )
                {
                    LOG_CLIENT( "Connection no endpoints" ) ;
                    THROW_RTE( "Failed to resolve ip: " << 
                        m_ip_address.value << " port: " << m_port_number );
                }

                m_endPoint = boost::asio::connect( m_socket, endpoints );
                m_socket_info = TCPSocketInfo::make( m_socket );
                LOG_CLIENT( "Connection ctor complete " << m_socket_info ) ;
            }

        private:
            void start()
            {
                VERIFY_RTE(!m_pReceiver);
                m_pReceiver = std::make_unique< SocketReceiver >(
                    shared_from_this(),
                    m_socket,
                    m_client.m_receiverCallback,
                    [ &client = m_client ]( service::Connection::WeakPtr pConnection )
                    {
                        client.onDisconnect( pConnection ); 
                    } );
            }
        public:
            const TCPSocketInfo& getSocketInfo() const override { return m_socket_info; }

            // connection is ALWAYS to a daemon so is always process ID zero.
            ProcessID getProcessID() const override { return PROCESS_ZERO; }

            void stop() override
            {
                if( m_pReceiver )
                {
                    m_pReceiver->stop();
                }
            }
            
            void send( const PacketBuffer& buffer ) override
            {
                if( m_socket.is_open() )
                {
                    m_sender.send( buffer );
                }
                else
                {
                    THROW_RTE( "Cannot send of disconnected connection" );
                }
            }

        private:
            Client&             m_client;
            IPAddress           m_ip_address;
            PortNumber          m_port_number;
            Resolver            m_resolver;
            Strand              m_strand;
            Socket              m_socket;
            TCPSocketInfo       m_socket_info;
            EndPoint            m_endPoint;
            SocketSender        m_sender;
            SocketReceiverPtr   m_pReceiver;
        };
        using ConnectionPtrSet = std::set< Connection::Ptr >;
        using ConnectionCallback = std::function< void(service::Connection::Ptr) >;
        using DisconnectCallback = std::function< void(service::Connection::Ptr) >;

        Client(boost::asio::io_context& io_context,
                ReceiverCallback receiverCallback,
                ConnectionCallback connectionCallback,
                DisconnectCallback disconnectCallback)
        :   m_io_context(io_context)
        ,   m_receiverCallback(std::move(receiverCallback))
        ,   m_connectionCallback(std::move(connectionCallback))
        ,   m_disconnectCallback(std::move(disconnectCallback))
        {
        }

        Connection::Ptr connect(IPAddress ip_address, PortNumber port_number)
        {
            auto pConnection = std::make_shared< Connection >(
                *this, ip_address, port_number );
            m_connections.insert(pConnection);
            m_connectionCallback(pConnection);
            pConnection->start();
            return pConnection;
        }

        void stop()
        {
            auto con = m_connections;
            for( auto c : con )
            {
                c->stop();
            }
        }
    private:
        void onDisconnect(service::Connection::WeakPtr pWeakConnectionPtr)
        {
            if( auto p = pWeakConnectionPtr.lock() )
            {
                Connection::Ptr pConnection =
                    std::dynamic_pointer_cast< Connection >( p );
                m_connections.erase(pConnection);
                m_disconnectCallback(pConnection);
            }
        }

    private:
        boost::asio::io_context&  m_io_context;
        ReceiverCallback          m_receiverCallback;
        ConnectionCallback        m_connectionCallback;
        DisconnectCallback        m_disconnectCallback;
        ConnectionPtrSet          m_connections;

    };
}

