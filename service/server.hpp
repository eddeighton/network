

#pragma once

#include "service/port_number.hpp"
#include "service/asio.hpp"
#include "service/receiver.hpp"
#include "service/sender_socket.hpp"
#include "service/socket_info.hpp"
#include "service/network.hpp"
#include "service/connection.hpp"

#include <boost/circular_buffer.hpp>

#include <set>
#include <iostream>

namespace mega::service
{
    class Server
    {
    public:
        class Connection : public service::Connection
        {
            friend class Server;
            
            using DisconnectCallback = std::function< void() >;
            using Strand             = boost::asio::strand< boost::asio::io_context::executor_type >;
            using Socket             = boost::asio::ip::tcp::socket;
            using SocketReceiver     = Receiver< Socket, DisconnectCallback >;

        public:
            using Ptr = std::shared_ptr< Connection >;
            using WeakPtr = std::weak_ptr< Connection >;

            Connection( Server& server, boost::asio::io_context& ioContext )
            : m_server( server )
            , m_strand( boost::asio::make_strand( ioContext ) )
            , m_socket( m_strand )
            , m_receiver( m_socket, server.m_receiverCallback, [ this ] { disconnected(); } )
            , m_sender( m_socket )
            , m_processID( PROCESS_ZERO )
            {
                m_waitForDisconnectFuture = m_waitForDisconnect.get_future();
            }

            ~Connection()
            {
                stop();
            }

            const TCPSocketInfo& getSocketInfo() const override { return m_socket_info; }

            Sender& getSender() override { return m_sender; }

            void stop() override
            {
                //std::cout << "Server connection stoppping " << m_socket_info << std::endl;
                if( m_receiver.started() && !m_bDisconnected )
                {
                    if( m_socket.is_open() )
                    {
                        boost::system::error_code ec;
                        m_socket.shutdown( m_socket.shutdown_both, ec );
                        m_socket.close( ec );
                    }
                    m_receiver.stop();
                    m_waitForDisconnectFuture.get();
                }
                //std::cout << "Server connection stop complete " << m_socket_info << std::endl;
            }

            ProcessID getProcessID() const override { return m_processID; }
        private:
            void start(ProcessID processID)
            {
                m_processID = processID;
                m_socket_info = TCPSocketInfo::make( m_socket );
                //std::cout << "Server connection start " << m_socket_info << std::endl;
                m_receiver.run(shared_from_this());
            }

            void disconnected()
            {
                //std::cout << "Server connection disconnecting " << m_socket_info << std::endl;
                if( m_socket.is_open() )
                {
                    boost::system::error_code ec;
                    m_socket.shutdown( m_socket.shutdown_both, ec );
                    m_socket.close( ec );
                }
                // NOTE - call to onDisconnect will invoke 
                // destructor which will block on m_bDisconnected being true
                if( !m_bDisconnected )
                {
                    m_bDisconnected = true;
                    m_waitForDisconnect.set_value();
                    // NOTE: deleted on next line
                    m_server.onDisconnect( 
                        std::dynamic_pointer_cast< Connection >( shared_from_this() ) );
                }

                //std::cout << "Server connection disconnect complete" << std::endl;
            }
        private:
            boost::fibers::promise<void>    m_waitForDisconnect;
            boost::fibers::future<void>     m_waitForDisconnectFuture;
            Server&         m_server;
            Strand          m_strand;
            Socket          m_socket;
            TCPSocketInfo   m_socket_info;
            SocketReceiver  m_receiver;
            SocketSender    m_sender;
            ProcessID       m_processID;
            bool            m_bDisconnected = false;
        };

        using ConnectionPtrMap = std::map< ProcessID, Connection::Ptr >;
        using ConnectionCallback = std::function< void(service::Connection::Ptr) >;
        using DisconnectCallback = std::function< void(service::Connection::Ptr) >;
        using ProcessIDFreeList = boost::circular_buffer< ProcessID >;

        Server(boost::asio::io_context& ioContext,
                PortNumber port_number,
                ReceiverCallback receiverCallback,
                ConnectionCallback connectionCallback,
                DisconnectCallback disconnectCallback
                )
        :   m_ioContext(ioContext)
        ,   m_port_number(port_number)
        ,   m_receiverCallback(std::move(receiverCallback))
        ,   m_connectionCallback(std::move(connectionCallback))
        ,   m_disconnectCallback(std::move(disconnectCallback))
        ,   m_acceptor( m_ioContext,
                boost::asio::ip::tcp::endpoint( boost::asio::ip::tcp::v4(), port_number.value ) )
        ,   m_processIDFreeList( std::numeric_limits< ProcessID::ValueType >::max() - 1 )
        {
            // simply add ALL valid ProcessIDs to the freelist on startup
            for( int i = 1; i != std::numeric_limits< ProcessID::ValueType >::max(); ++i )
            {
                m_processIDFreeList.push_back(
                    ProcessID{ static_cast< ProcessID::ValueType >(i) } );
            }
        }

        ~Server()
        {
            stop();
        }

        void start()
        {
            waitForConnection();
        }

        void stop()
        {
            m_acceptor.close();
            auto con = m_connections;
            for( auto& c : con )
            {
                c.second->stop();
            }
        }

    private:
        void waitForConnection()
        {
            boost::fibers::fiber([this]
            {
                //std::cout << "Acceptor started" << std::endl;
                if( m_acceptor.is_open() )
                {
                    Connection::Ptr pNewConnection =
                        std::make_shared< Connection >( *this, m_ioContext );
                    boost::system::error_code ec;
                    m_acceptor.async_accept( pNewConnection->m_socket,
                        boost::fibers::asio::yield[ ec ]);
                    onConnect( pNewConnection, ec );
                }
                //std::cout << "Acceptor stopped" << std::endl;

            }).detach();
        }

        void onConnect( Connection::Ptr pNewConnection, const boost::system::error_code& ec )
        {
            if( !ec )
            {
                VERIFY_RTE_MSG(!m_processIDFreeList.empty(),
                    "No free ProcessIDs available" );

                const auto processID = m_processIDFreeList.front();
                m_processIDFreeList.pop_front();
                pNewConnection->start(processID);
                m_connections.insert(std::make_pair(processID, pNewConnection));
                m_connectionCallback(pNewConnection);
            }
            if( m_acceptor.is_open() )
            {                   
                waitForConnection();
            }
        }

        void onDisconnect( Connection::Ptr pConnection )
        {
            // std::cout << "onDisconnect" << std::endl;
            auto processID = pConnection->getProcessID();
            auto iFind = m_connections.find(processID);
            VERIFY_RTE_MSG( iFind != m_connections.end(),
                "Invalid call to onDisconnect for already disconnected connection");
 
            m_connections.erase( iFind );
            m_processIDFreeList.push_back( processID );
            m_disconnectCallback( pConnection );
        }

        boost::asio::io_context&        m_ioContext;
        PortNumber                      m_port_number;
        ReceiverCallback                m_receiverCallback;
        ConnectionCallback              m_connectionCallback;
        DisconnectCallback              m_disconnectCallback;
        boost::asio::ip::tcp::acceptor  m_acceptor;
        ConnectionPtrMap                m_connections;
        ProcessIDFreeList               m_processIDFreeList;
    };
}

