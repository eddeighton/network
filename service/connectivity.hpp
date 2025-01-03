
#pragma once
// Sun Dec 29 13:34:52 UTC 2024

#include "service/interface/connectivity.interface.hpp"

#include "service/registry.hpp"
#include "service/daemon.hpp"

#include <iostream>

namespace mega::service
{
    class OConnectivity : public Connectivity
    {
        service::MPTFO m_mptfo;
        service::Ptr< Connectivity > m_pProxy;
        service::Daemon& m_daemon;
    public:
        OConnectivity(service::Daemon& daemon)
        :   m_daemon(daemon)
        {
            auto reg = service::Registry::getWriteAccess();
            m_mptfo = reg->createInProcessProxy(service::LogicalThread::get().getMPTF(), *this);
            m_pProxy = reg->one< Connectivity >(m_mptfo);
        }
        void shutdown() override
        {
            std::cout << "shutdown received" << std::endl;
            m_daemon.shutdown();
        }
    };
}

