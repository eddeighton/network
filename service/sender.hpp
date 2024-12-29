
#pragma once


#include "service/protocol/message.hpp"
#include "service/fibers.hpp"

#include "service/protocol/packet.hpp"

#include <boost/asio/ip/tcp.hpp>
#include <boost/system/error_code.hpp>

#include <memory>

namespace mega::service
{

class Sender
{
public:
    virtual ~Sender() = 0;
    virtual boost::system::error_code send( const PacketBuffer& msg ) = 0;
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

    boost::system::error_code send( const PacketBuffer& payload ) final
    {
        return service::send( m_socket, payload );
    }

private:
    Socket& m_socket;
};

}

