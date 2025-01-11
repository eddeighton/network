#pragma once

#include "service/base_interfaces.hpp"

#include <string>

namespace mega::service
{
struct Python : public virtual Interface
{
    virtual void loadScript( std::string strPythonScriptPath ) = 0;
    virtual void reload()                                      = 0;
    virtual void unload()                                      = 0;
};
} // namespace mega::service
