

#pragma once

#include "service/port_number.hpp"
#include "service/asio.hpp"
#include "service/receiver.hpp"
#include "service/sender_socket.hpp"
#include "service/socket_info.hpp"
#include "service/network.hpp"

#include <set>
#include <iostream>

namespace mega::service
{

    class Server
    {
    public:
        class Connection : public std::enable_shared_from_this< Connection >
        {
            friend class Server;
            
            using DisconnectCallback = std::function< void() >;
            using Strand             = boost::asio::strand< boost::asio::io_context::executor_type >;
            using Socket             = boost::asio::ip::tcp::socket;
            using SocketReceiver     = Receiver< Socket, DisconnectCallback >;

        public:
            using Ptr = std::shared_ptr< Connection >;

            Connection( Server& server, boost::asio::io_context& ioContext )
            : m_server( server )
            , m_strand( boost::asio::make_strand( ioContext ) )
            , m_socket( m_strand )
            , m_receiver( m_socket, server.m_receiverCallback, [ this ] { disconnected(); } )
            , m_sender( m_socket )
            {
            }

            Sender& getSender() { return m_sender; }
        private:
            void start()
            {
                m_socket_info = TCPSocketInfo::make( m_socket );
                std::cout << "Server connection start " << m_socket_info << std::endl;
                m_receiver.run();
                std::cout << "Connect" << std::endl;
            }

            void stop()
            {
                std::cout << "Server connection stop " << m_socket_info << std::endl;
                m_socket.shutdown( Socket::shutdown_both );
                m_receiver.stop();
            }

            void disconnected()
            {
                std::cout << "Server connection disconnect " << m_socket_info << std::endl;
                if( m_socket.is_open() )
                {
                    boost::system::error_code ec;
                    m_socket.shutdown( m_socket.shutdown_both, ec );
                    m_socket.close( ec );
                }
                // if( m_disconnectCallback.has_value() )
                // {
                //     ( *m_disconnectCallback )();
                // }
                m_server.onDisconnect( shared_from_this() );
                std::cout << "Disconnect" << std::endl;
            }

        private:
            Server&         m_server;
            Strand          m_strand;
            Socket          m_socket;
            TCPSocketInfo   m_socket_info;
            SocketReceiver  m_receiver;
            SocketSender    m_sender;
        };

        using ConnectionPtrSet = std::set< Connection::Ptr >;

        Server(Network& network, PortNumber port_number, ReceiverCallback receiverCallback)
        :   m_ioContext(network.getIOContext())
        ,   m_port_number(port_number)
        ,   m_receiverCallback(std::move(receiverCallback))
        ,   m_acceptor( m_ioContext,
                boost::asio::ip::tcp::endpoint( boost::asio::ip::tcp::v4(), port_number.value ) )
        {
            boost::fibers::fiber([this]
            {
                waitForConnection();
            }).detach();
        }
        Server(boost::asio::io_context& ioContext, PortNumber port_number, ReceiverCallback receiverCallback)
        :   m_ioContext(ioContext)
        ,   m_port_number(port_number)
        ,   m_receiverCallback(std::move(receiverCallback))
        ,   m_acceptor( m_ioContext,
                boost::asio::ip::tcp::endpoint( boost::asio::ip::tcp::v4(), port_number.value ) )
        {
            boost::fibers::fiber([this]
            {
                waitForConnection();
            }).detach();
        }

    private:
        void waitForConnection()
        {
            Connection::Ptr pNewConnection =
                std::make_shared< Connection >( *this, m_ioContext );

            boost::system::error_code ec;
            m_acceptor.async_accept( pNewConnection->m_socket,
                boost::fibers::asio::yield[ ec ]);
 
            onConnect( pNewConnection, ec );
            //m_acceptor.async_accept( pNewConnection->m_socket,
           //     boost::asio::bind_executor( pNewConnection->m_strand,
           //         boost::bind( &Server::onConnect, this, pNewConnection,
           //             boost::asio::placeholders::error ) ) );
        }


        void onConnect( Connection::Ptr pNewConnection, const boost::system::error_code& ec )
        {
            if( !ec )
            {
                pNewConnection->start();
                m_connections.insert( pNewConnection );
            }
            if( m_acceptor.is_open() )
            {
                waitForConnection();
            }
        }

        void onDisconnect( Connection::Ptr pConnection )
        {
            m_connections.erase( pConnection );
        }

        boost::asio::io_context&        m_ioContext;
        PortNumber                      m_port_number;
        ReceiverCallback                m_receiverCallback;
        boost::asio::ip::tcp::acceptor  m_acceptor;
        ConnectionPtrSet                m_connections;
    };
}

