
#pragma once
// Sun Dec 29 05:23:02 UTC 2024

#include "service/base_interfaces.hpp"
#include "service/ptr.hpp"

#include <string>
#include <memory>
#include <vector>

namespace mega
{
    namespace service
    {
        struct Connectivity : public virtual mega::service::Daemon
        {
            //virtual void connect() = 0;
            //virtual void disconnect() = 0;
            virtual void shutdown() = 0;
        };
    }
}

