
#pragma once
// Thu Jan  9 20:26:56 UTC 2025

#include "service/base_interfaces.hpp"
#include "service/ptr.hpp"

#include "vocab/service/keyboard_event.hpp"

namespace mega
{
namespace controller
{
struct Controller : public virtual service::Interface
{
    virtual bool onKeyboardEvent( KeyboardEvent ev ) = 0;
};

struct ControllerFactory : public virtual mega::service::Factory
{
    virtual service::Ptr< Controller > create_controller() = 0;
};
} // namespace test
} // namespace mega
