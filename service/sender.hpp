
#pragma once


#include "service/message.hpp"
#include "service/asio.hpp"

#include <boost/system/error_code.hpp>

#include <memory>

namespace service
{

class Sender
{
public:
    virtual ~Sender() = 0;
    virtual boost::system::error_code send( const Message& msg ) = 0;
};
inline Sender::~Sender() = default;

class SocketSender : public Sender
{
public:
    using Socket = boost::asio::ip::tcp::socket;

    SocketSender( Socket& socket )
        : m_socket( socket )
    {
    }

    boost::system::error_code send( const Message& msg ) final
    {
        boost::system::error_code ec;

        const auto szBytesWritten
            = boost::asio::async_write( m_socket,
                boost::asio::buffer( msg ),
                boost::fibers::asio::yield[ ec ]);
        if( !ec )
        {
            // VERIFY_RTE( szBytesWritten == msg.size() );
        }
        return ec;
    }

private:
    Socket& m_socket;
};

}

