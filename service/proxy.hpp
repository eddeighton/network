
#pragma once

#include "service/rtti.hpp"

namespace mega::service
{

    template< typename T >
    class Proxy : public T
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

