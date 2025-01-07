
#pragma once

#include "service/access.hpp"
#include "service/registry.hpp"
#include "service/ptr.hpp"
#include "service/protocol/serialization.hpp"

namespace mega::service::detail
{
    template< typename T, typename Archive >
    void serialize( std::weak_ptr< Proxy< T > >& pointer, Archive& archive )
    {
        using ProxyType = Proxy< T >;
        using PointerType = std::weak_ptr< Proxy< T > >;

        if constexpr( boost::serialization::IsXMLArchive< Archive >::value )
        {
            //archive& boost::serialization::make_nvp( "machine_id", value );
            if constexpr( Archive::is_saving::value )
            {
                THROW_TODO;
            }
            else
            {
                THROW_TODO;
            }
        }
        else
        {
            if constexpr( Archive::is_saving::value )
            {
                if( auto p = pointer.lock() )
                {
                    archive << true;
                    archive << p->getMPTFO();
                    archive << getInterfaceTypeName< T >();
                }
                else
                {
                    archive << false;
                }
            }
            else
            {
                bool bNonNull = false;
                archive >> bNonNull;
                if( bNonNull )
                {
                    MPTFO mptfo{};
                    archive >> mptfo;

                    IArchive& megaArchive = dynamic_cast< IArchive& >( archive );
                    pointer = megaArchive.access().readRegistry()->template one<T>(mptfo);
                }
            }
        }
    }
}

