
#pragma once
// Sun Dec 29 13:34:52 UTC 2024

#include "service/interface/test.interface.hpp"

#include "service/network.hpp"
#include "service/registry.hpp"

#include <iostream>

namespace mega::test
{
    class OConnectivity : public Connectivity
    {
        service::MPO m_mpo;
        service::Ptr< Connectivity > m_pProxy;
    public:
        OConnectivity(service::Network& network)
        {
            auto& reg = network.writeRegistry().get();
            m_mpo = reg.createInProcessProxy(*this);
            m_pProxy = reg.one< Connectivity >(m_mpo);
        }
        void shutdown() override
        {
            std::cout << "shutdown received" << std::endl;
        }
    };
}

