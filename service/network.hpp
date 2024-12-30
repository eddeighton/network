
#pragma once

#include "service/registry.hpp"
#include "service/asio.hpp"

#include "vocab/service/mp.hpp"

#include <mutex>
#include <shared_mutex>
#include <thread>

namespace mega::service
{
    class Network
    {
        std::shared_mutex m_mutex;
        Registry m_registry;
        mega::service::IOContextPtr m_pIOContext;
        std::thread m_networkThread;

        void run()
        {
            mega::service::init_fiber_scheduler(m_pIOContext);
            m_pIOContext->run();
        }
    public:
        Network(MP machineProcess)
        :   m_registry(machineProcess)
        ,   m_pIOContext(std::make_shared< boost::asio::io_context >())
        ,   m_networkThread( [this]
            {
                run();    
            })
        {
        }

        Network(const Network&) = delete;
        Network& operator=(const Network&) = delete;

        ~Network()
        {
            m_pIOContext->stop();
            m_networkThread.join();
        }

        MP getMP() const { return m_registry.getMP(); }
        auto& getIOContext() { return *m_pIOContext; }

        struct RegistryReadAccess
        {
            std::shared_mutex& m_mutex;
            std::shared_lock< std::shared_mutex > m_shared_lock;
            Registry& m_registry;

            RegistryReadAccess(
                std::shared_mutex& mut,
                Registry& reg )
                : m_mutex( mut )
                , m_shared_lock( m_mutex, std::defer_lock )
                , m_registry( reg )
            {
            }

            RegistryReadAccess(const RegistryReadAccess&)=delete;
            RegistryReadAccess(RegistryReadAccess&&)=default;
            RegistryReadAccess& operator=(const RegistryReadAccess&)=delete;
            RegistryReadAccess& operator=(RegistryReadAccess&&)=default;

            Registry& get()
            {
                // lock the reader on access
                m_shared_lock.lock();
                return m_registry;
            }
        };
        struct RegistryWriteAccess
        {
            std::shared_mutex& m_mutex;
            std::lock_guard< std::shared_mutex > m_lock_guard;
            Registry& m_registry;

            RegistryWriteAccess(
                std::shared_mutex& mut,
                Registry& reg )
                : m_mutex( mut )
                , m_lock_guard( m_mutex )
                , m_registry( reg )
            {
            }

            RegistryWriteAccess(const RegistryWriteAccess&)=delete;
            RegistryWriteAccess(RegistryWriteAccess&&)=default;
            RegistryWriteAccess& operator=(const RegistryWriteAccess&)=delete;
            RegistryWriteAccess& operator=(RegistryWriteAccess&&)=default;
            
            Registry& get()
            {
                return m_registry;
            }
        };

        RegistryReadAccess readRegistry()
        {
            return {m_mutex, m_registry};
        }
        RegistryWriteAccess writeRegistry()
        {
            return {m_mutex, m_registry};
        }

    };
}

