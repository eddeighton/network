
#pragma once

#include "service/registry.hpp"
#include "service/asio.hpp"

#include "vocab/service/mp.hpp"

#include <thread>

namespace mega::service
{
    class Network
    {
        mega::service::IOContextPtr m_pIOContext;
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

        inline Network(const Network&) = delete;
        inline Network& operator=(const Network&) = delete;

        inline ~Network()
        {
            m_pIOContext->stop();
            m_networkThread.join();
        }

        inline auto& getIOContext() { return *m_pIOContext; }

        inline void shutdown()
        {
            // mega::service::LogicalThread::get().stop();
            m_pIOContext->stop();
        }
    };
}

