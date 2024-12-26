
#pragma once

#include <string>
#include <vector>

namespace mega::service
{

    class RTTI
    {
    public:
        using InterfaceTypeName = std::string;
        using InterfaceTypeNameVector = std::vector< InterfaceTypeName >;


    private:
        InterfaceTypeNameVector m_interfaces;
    };

    template< typename T >
    RTTI getRTTI( const T* pPtr )
    {
        RTTI rtti;

        // attempt all dynamic casts
        // if( dynamic_cast< foobar >( pPtr ) )
        // {
        //     rtti.addInterface( "foobar" );
        // }

        return rtti;
    }

}

