
#pragma once

#include "service/interface/python.interface.hpp"
#include "controller/interface/controller.interface.hpp"

#include "service/access.hpp"
#include "service/registry.hpp"

#include "common/log.hpp"
#include "common/visibility.hpp"

#include <pybind11/embed.h>

#include <memory>

namespace mega::controller
{

// using namespace pybind11::literals;
class VISIBILITY_HIDDEN OController : public Controller,
                                      public service::Python
{
    service::Access& m_access;
    service::MPTFO   m_mptfo;

    using PythonPtr = std::unique_ptr< pybind11::scoped_interpreter >;
    PythonPtr m_pPythonGuard;

public:
    OController( service::Access& access )
        : m_access( access )
    {
        auto reg = m_access.writeRegistry();
        m_mptfo  = reg->createInProcessProxy(
            service::LogicalThread::get().getMPTF(), *this );
    }

    bool onKeyboardEvent( KeyboardEvent ev )
    {
        LOG( "Controller::onKeyboardEvent called: "
             << ev.key << " down: " << ev.down );

        return false;
    }
    void loadScript( std::string strPythonScriptPath ) override
    {
        //
    }
    void reload() override
    {
        //
    }
    void unload() override
    {
        //
    }
};
} // namespace mega::controller
