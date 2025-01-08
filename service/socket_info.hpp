#pragma once

#include "service/asio.hpp"

#include <iostream>

namespace mega::service
{
struct TCPSocketInfo
{
    using EndPointType = boost::asio::ip::tcp::socket::endpoint_type;

    EndPointType local_endpoint;
    EndPointType remote_endpoint;

    static inline TCPSocketInfo
    make( const boost::asio::ip::tcp::socket& socket )
    {
        return { socket.local_endpoint(), socket.remote_endpoint() };
    }
};

inline std::ostream& operator<<( std::ostream&        os,
                                 const TCPSocketInfo& info )
{
    return os << "Local Endpoint: " << info.local_endpoint
              << " Remote Endpoint: " << info.remote_endpoint;
}
} // namespace mega::service
