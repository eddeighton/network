
#pragma once

#include "service/rtti.hpp"
#include "service/ptr.hpp"

namespace mega::service
{
    class Registration
    {
    public:
        Registration()
        {
        }
        
        template< typename T >
        void register_ptr( Ptr< T >& ptr )
        {

        }
    };
}

