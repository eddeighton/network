
#pragma once

#include "controller/interface/controller.interface.hpp"

#include "service/access.hpp"
#include "service/registry.hpp"

#include "common/log.hpp"

namespace mega::controller
{

class OController : public Controller
{
    service::Access& m_access;
    service::MPTFO   m_mptfo;

public:
    OController( service::Access& access )
        : m_access( access )
    {
        auto reg = m_access.writeRegistry();
        m_mptfo  = reg->createInProcessProxy(
            service::LogicalThread::get().getMPTF(), *this );
    }
    virtual bool onKeyboardEvent( KeyboardEvent ev ) override
    {
        LOG( "Controller::onKeyboardEvent called: "
             << ev.key << " down: " << ev.down );

        return false;
    }
};
} // namespace mega::controller
