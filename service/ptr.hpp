
#pragma once

#include "service/proxy.hpp"

#include "common/assert_verify.hpp"
#include "common/serialisation.hpp"

namespace mega::service
{
    template< typename T >
    class Ptr
    {
        using ProxyType = Proxy< T >;
        ProxyType* p = nullptr;
    public:
        Ptr() = default;
        Ptr(ProxyType* p) : p(p) {}

        explicit operator bool() const { return p; }
        ProxyType* operator->() const { return p; }
        ProxyType& operator*() const { ASSERT(p); return *p; }

        std::string str() const
        {
            std::ostringstream os;
            if( p )
            {
                os << p->getMPTFO();
            }
            return os.str();
        }

        template < class Archive >
        inline void serialize( Archive& archive, const unsigned int )
        {
            if constexpr( boost::serialization::IsXMLArchive< Archive >::value )
            {
                //archive& boost::serialization::make_nvp( "machine_id", value );
            }
            else
            {
                //archive& value;
            }
        }

    };
}
