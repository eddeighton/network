
#pragma once

// Sun Jan 12 10:09:04 UTC 2025

#include "service/base_interfaces.hpp"
#include "service/ptr.hpp"

namespace mega::controller
{

struct Neovim : public virtual service::Interface
{
    virtual void loadFileLine( std::string strFileName,
                               int         lineNumber )
        = 0;
};

struct NeovimFactory : public virtual mega::service::Factory
{
    virtual service::Ptr< Neovim >
    create_neovim( std::string strNeovimPipe ) = 0;
};
} // namespace mega::controller
