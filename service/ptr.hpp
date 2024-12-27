
#pragma once

#include "service/proxy.hpp"

#include "common/assert_verify.hpp"

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
    };
}

