
#pragma once

#include "service/proxy.hpp"

#include "common/assert_verify.hpp"
#include "common/serialisation.hpp"

#include <memory>

namespace mega::service
{
    namespace detail
    {
        template< typename T, typename Archive >
        void serialize( std::weak_ptr< Proxy< T > >& pointer, Archive& archive );
    }

    template< typename T >
    class Ptr
    {
        using ProxyType = Proxy< T >;
        using PointerType = std::weak_ptr< Proxy< T > >;
        mutable PointerType m_ptr;

        ProxyType* get() const { return m_ptr.lock().get(); }
    public:
        Ptr() = default;
        Ptr(std::shared_ptr< Proxy< T > > p) : m_ptr(p) {}

        explicit operator bool() const { return get(); }
        ProxyType* operator->() const { return get(); }
        ProxyType& operator*() const { auto p = get(); ASSERT(p); return *p; }

        std::string str() const
        {
            std::ostringstream os;
            if( auto p = get() )
            {
                os << p->getMPTFO();
            }
            else
            {
                os << "nullptr";
            }
            return os.str();
        }

        template < class Archive >
        void serialize( Archive& archive, const unsigned int )
        {    
            detail::serialize< T, Archive >( m_ptr, archive );
        }
    };
}

