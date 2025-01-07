
#pragma once

#include "service/registry.hpp"
#include "service/asio.hpp"

#include "vocab/service/mp.hpp"

#include <thread>
#include <atomic>

namespace mega::service
{
    class Network
    {
        IOContextPtr m_pIOContext;
        std::thread m_networkThread;

        inline void run()
        {
            mega::service::init_fiber_scheduler(m_pIOContext);
            m_pIOContext->run();
        }
    public:
        inline Network()
        :   m_pIOContext(std::make_shared< boost::asio::io_context >())
        ,   m_networkThread( [this]
            {
                run();    
            })
        {
        }

        inline ~Network()
        {
            m_pIOContext->stop();
            m_networkThread.join();
        }

        inline Network(const Network&) = delete;
        inline Network& operator=(const Network&) = delete;

        inline auto& getIOContext() { return *m_pIOContext; }
    };
}

