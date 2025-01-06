
#pragma once

#include "service/port_number.hpp"
#include "service/ip_address.hpp"
#include "service/asio.hpp"
#include "service/receiver.hpp"
#include "service/sender_socket.hpp"
#include "service/socket_info.hpp"
#include "service/network.hpp"
#include "service/connection.hpp"

#include <set>
#include <iostream>

namespace mega::service
{
    class Client
    {
    public:        
        class Connection : public service::Connection
        {
            friend class Client;

            using DisconnectCallback = std::function< void() >;
            using Strand             = boost::asio::strand< boost::asio::io_context::executor_type >;
            using Socket             = boost::asio::ip::tcp::socket;
            using Resolver           = boost::asio::ip::tcp::resolver;
            using EndPoint           = boost::asio::ip::tcp::endpoint;
            using SocketReceiver     = Receiver< Socket, DisconnectCallback >;
        public:
            using Ptr = std::shared_ptr< Connection >;

            Connection(Client& client, IPAddress ip_address, PortNumber port_number)
            :   m_client(client)
            ,   m_ip_address(std::move(ip_address))
            ,   m_port_number(std::move(port_number))
            ,   m_resolver(client.m_io_context)
            ,   m_strand( boost::asio::make_strand( client.m_io_context ) )
            ,   m_socket( m_strand )
            ,   m_receiver( m_socket, client.m_receiverCallback, [ this ] { disconnected(); } )
            ,   m_sender( m_socket )
            {
                Resolver::query        query( m_ip_address.value, m_port_number.str() );
                Resolver::results_type endpoints = m_resolver.resolve( query );

                if( endpoints.empty() )
                {
                    //SPDLOG_ERROR( "Failed to resolve ip: {} port: {}", strServiceIP, portNumber );
                    //THROW_RTE( "Failed to resolve ip: " << strServiceIP << " port: " << portNumber );
                    throw std::runtime_error("Failed to locate ip address");
                }

                m_endPoint = boost::asio::connect( m_socket, endpoints );
                m_receiver.run();

                m_socket_info = TCPSocketInfo::make( m_socket );
                // std::cout << "Client connection start " << m_socket_info << std::endl;
            }

            const TCPSocketInfo& getSocketInfo() const override { return m_socket_info; }

            Sender& getSender() override { return m_sender; }

            void stop() override
            {
                // std::cout << "Client connection stop from: " << m_socket_info << std::endl;
                boost::system::error_code ec;
                m_socket.shutdown( m_socket.shutdown_both, ec );
                m_socket.close();

                while(m_bDisconnected == false)
                {
                    boost::this_fiber::yield();
                }
            }
            
            // connection is ALWAYS to a daemon so is always process ID zero.
            ProcessID getProcessID() const override { return PROCESS_ZERO; }
        private:
            void disconnected()
            {
                // std::cout << "Client connection disconnect from: " << m_socket_info << std::endl;
                m_client.onDisconnect(
                    std::dynamic_pointer_cast< Connection >( shared_from_this() ));
                // std::cout << "Disconnected" << std::endl;
                m_bDisconnected = true;
            }

        private:
            Client&         m_client;
            IPAddress       m_ip_address;
            PortNumber      m_port_number;
            Resolver        m_resolver;
            Strand          m_strand;
            Socket          m_socket;
            TCPSocketInfo   m_socket_info;
            EndPoint        m_endPoint;
            SocketReceiver  m_receiver;
            SocketSender    m_sender;
            bool            m_bDisconnected = false;
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
            auto pConnection = std::make_shared< Connection >(*this, ip_address, port_number );
            m_connections.insert(pConnection);
            m_connectionCallback(pConnection);
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
        void onDisconnect(Connection::Ptr pConnection)
        {
            m_connections.erase(pConnection);
            m_disconnectCallback(pConnection);
        }

    private:
        boost::asio::io_context&  m_io_context;
        ReceiverCallback          m_receiverCallback;
        ConnectionCallback        m_connectionCallback;
        DisconnectCallback        m_disconnectCallback;
        ConnectionPtrSet          m_connections;

    };
}

