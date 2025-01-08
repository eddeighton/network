
#pragma once
// Sun Dec 29 05:23:02 UTC 2024

#include "service/base_interfaces.hpp"

namespace mega
{
namespace service
{
struct Connectivity : public virtual mega::service::Interface
{
    // virtual void connect() = 0;
    // virtual void disconnect() = 0;
    virtual void shutdown() = 0;
};
} // namespace service
} // namespace mega
