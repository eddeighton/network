
#pragma once

#include "service/asio.hpp"
#include "service/connection.hpp"

#include "common/log.hpp"

#include <functional>
#include <memory>

// using namespace std::string_literals;
// #define LOG_ACCEPTOR(msg) LOG("ACCEPTOR: "s + msg)
#define LOG_ACCEPTOR( msg )

namespace mega::service
{
template < typename TConnectionPtr >
class Acceptor
{
public:
    using Ptr = std::unique_ptr< Acceptor >;

    using ConnectionFactory = std::function< TConnectionPtr() >;
    using ConnectionCallback
        = std::function< void( TConnectionPtr ) >;

    Acceptor( boost::asio::io_context&       ioContext,
              boost::asio::ip::tcp::endpoint endpoint,
              ConnectionFactory              connectionFactory,
              ConnectionCallback             connectionCallback )
        : m_acceptor( ioContext, endpoint )
        , m_connectionFactory( std::move( connectionFactory ) )
        , m_connectionCallback( std::move( connectionCallback ) )
        , m_fiber( [ this ]() { run(); } )
    {
        LOG_ACCEPTOR( "Acceptor ctor" );
    }

    ~Acceptor()
    {
        LOG_ACCEPTOR( "Acceptor dtor start" );
        m_acceptor.close();
        m_fiber.join();
        LOG_ACCEPTOR( "Acceptor dtor complete" );
    }

private:
    void run()
    {
        LOG_ACCEPTOR( "Acceptor fiber started" );
        while( m_acceptor.is_open() )
        {
            TConnectionPtr pNewConnection = m_connectionFactory();
            boost::system::error_code ec;
            m_acceptor.async_accept(
                pNewConnection->m_socket,
                boost::fibers::asio::yield[ ec ] );
            if( !ec )
            {
                LOG_ACCEPTOR( "Starting new connection" );
                m_connectionCallback( pNewConnection );
            }
            else
            {
                LOG_ACCEPTOR( "Error returned from aync_accept" );
            }
        }
        LOG_ACCEPTOR( "Acceptor fiber stopped" );
    }

    boost::asio::ip::tcp::acceptor m_acceptor;
    ConnectionFactory              m_connectionFactory;
    ConnectionCallback             m_connectionCallback;
    boost::fibers::fiber           m_fiber;
};

} // namespace mega::service
