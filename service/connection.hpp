
// Sat Jan  4 16:50:20 UTC 2025
#pragma once

#include "service/socket_info.hpp"

#include <memory>

namespace mega::service
{
    class Connection : public std::enable_shared_from_this< Connection >
    {
    public:
        using Ptr = std::shared_ptr< Connection >;
        using WeakPtr = std::weak_ptr< Connection >;

        virtual ~Connection() = 0;
        virtual const TCPSocketInfo& getSocketInfo() const = 0;
        virtual ProcessID getProcessID() const = 0;
        virtual Sender& getSender() = 0;
        virtual void stop() = 0;
    };
    inline Connection::~Connection() = default;
}

