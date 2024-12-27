
#pragma once

#include "service/rtti.hpp"
#include "service/proxy.hpp"

namespace mega::service
{
    class Registration
    {
    public:
        Registration()
        {
        }
        
        template< typename T >
        void registerProxy(Proxy< T >& proxy)
        {

        }
    };
}

