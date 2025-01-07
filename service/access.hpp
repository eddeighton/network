
#pragma once

#include "service/registry.hpp"

#include "common/disable_special_members.hpp"

#include <mutex>
#include <shared_mutex>

namespace mega::service
{
    class Access : public Common::DisableCopy, Common::DisableMove
    {
        std::shared_mutex m_mutex;
        Registry m_registry;
    public:
        Access()
            : m_registry( *this )
        {

        }
        
        class RegistryReadAccess
        {
            std::shared_mutex& m_mutex;
            std::shared_lock< std::shared_mutex > m_shared_lock;
            Registry& m_registry;
        public:
            RegistryReadAccess(std::shared_mutex& mut, Registry& reg)
                : m_mutex( mut )
                , m_shared_lock( m_mutex )
                , m_registry( reg )
            {
            }

            RegistryReadAccess(const RegistryReadAccess&)=delete;
            RegistryReadAccess(RegistryReadAccess&&)=default;
            RegistryReadAccess& operator=(const RegistryReadAccess&)=delete;
            RegistryReadAccess& operator=(RegistryReadAccess&&)=default;

            const Registry* operator->() { return &m_registry; }
        };

        class RegistryWriteAccess
        {
            std::shared_mutex& m_mutex;
            std::lock_guard< std::shared_mutex > m_lock_guard;
            Registry& m_registry;
        public:
            RegistryWriteAccess(std::shared_mutex& mut, Registry& reg)
                : m_mutex( mut )
                , m_lock_guard( m_mutex )
                , m_registry( reg )
            {
            }

            RegistryWriteAccess(const RegistryWriteAccess&)=delete;
            RegistryWriteAccess(RegistryWriteAccess&&)=default;
            RegistryWriteAccess& operator=(const RegistryWriteAccess&)=delete;
            RegistryWriteAccess& operator=(RegistryWriteAccess&&)=default;
            
            Registry* operator->() { return &m_registry; }
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

#include "service/ptr.ipp"

