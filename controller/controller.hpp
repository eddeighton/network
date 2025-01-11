
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

    using ModulePtr = std::unique_ptr< pybind11::module >;
    ModulePtr m_pModule;

public:
    OController( service::Access& access )
        : m_access( access )
    {
        auto reg = m_access.writeRegistry();
        m_mptfo  = reg->createInProcessProxy(
            service::LogicalThread::get().getMPTF(), *this );
    }

    bool onKeyboardEvent( KeyboardEvent ev ) override
    {
        LOG( "Controller::onKeyboardEvent called: "
             << ev.key << " down: " << ev.down );
        if( m_pModule )
        {
            pybind11::object result
                = m_pModule->attr( "onEvent" )( ev.key, ev.down );
            return result.cast< bool >();
        }
        return false;
    }
    void loadScript( std::string strPythonScriptPath ) override
    {
        if( !m_pPythonGuard )
        {
            m_pPythonGuard
                = std::make_unique< pybind11::scoped_interpreter >();
        }
        m_pModule = std::make_unique< pybind11::module >(
            pybind11::module::import( strPythonScriptPath.c_str() ) );
        LOG( "Loaded python script: " << strPythonScriptPath );
    }
    void reload() override
    {
        VERIFY_RTE_MSG(
            m_pModule, "No module loaded in call to reload" );
        m_pModule->reload();
    }
    void unload() override
    {
        VERIFY_RTE_MSG(
            m_pModule, "No module loaded in call to reload" );
        m_pModule.reset();
    }
};
} // namespace mega::controller
