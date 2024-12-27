
#pragma once

#include "service/proxy.hpp"

namespace mega::service
{
    template< typename T >
    class Ptr
    {
        using ProxyType = Proxy< T >;
        ProxyType* p;
    public:
        Ptr() : p(nullptr) {}
        Ptr(ProxyType* p) : p(p) {}

        explicit operator bool() const { return p; }
        ProxyType* operator->() const { return p; }
        ProxyType& operator*() const { return *p; }
    };
}

