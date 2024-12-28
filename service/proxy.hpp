
#pragma once

#include "service/rtti.hpp"

#include "common/disable_special_members.hpp"

namespace mega::service
{
    template< typename T >
    class Proxy : public T, Common::DisableCopy, Common::DisableMove
    {
    protected:
        RTTI m_rtti;
    protected:
        Proxy( const RTTI& rtti )
        :   m_rtti( rtti )
        {
        }
        Proxy(Proxy&)=delete;
    };
}

