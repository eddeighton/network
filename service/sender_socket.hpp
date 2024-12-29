
#pragma once

#include "service/sender.hpp"

#include "service/protocol/packet.hpp"

#include <boost/asio/ip/tcp.hpp>
#include <boost/system/error_code.hpp>

namespace mega::service
{
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

