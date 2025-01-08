
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
    Registry          m_registry;

public:
    Access()
        : m_registry( *this )
    {
    }

    Access( const Access& )            = delete;
    Access( Access&& )                 = delete;
    Access& operator=( const Access& ) = delete;
    Access& operator=( Access&& )      = delete;

    class RegistryReadAccess
    {
        std::shared_mutex&                    m_mutex;
        std::shared_lock< std::shared_mutex > m_shared_lock;
        Registry&                             m_registry;

    public:
        RegistryReadAccess( std::shared_mutex& mut, Registry& reg )
            : m_mutex( mut )
            , m_shared_lock( m_mutex )
            , m_registry( reg )
        {
        }

        RegistryReadAccess( const RegistryReadAccess& ) = delete;
        RegistryReadAccess( RegistryReadAccess&& )      = default;
        RegistryReadAccess& operator=( const RegistryReadAccess& )
            = delete;
        RegistryReadAccess& operator=( RegistryReadAccess&& other )
            = delete;

        const Registry* operator->() { return &m_registry; }
    };

    class RegistryWriteAccess
    {
        using LockType    = std::lock_guard< std::shared_mutex >;
        using LockTypePtr = std::unique_ptr< LockType >;

        std::shared_mutex& m_mutex;
        LockTypePtr        m_pLock;
        Registry&          m_registry;

    public:
        RegistryWriteAccess( std::shared_mutex& mut, Registry& reg )
            : m_mutex( mut )
            , m_pLock( std::make_unique< LockType >( m_mutex ) )
            , m_registry( reg )
        {
        }
        RegistryWriteAccess( RegistryWriteAccess&& other ) = default;

        RegistryWriteAccess& operator=( RegistryWriteAccess&& )
            = delete;
        RegistryWriteAccess( const RegistryWriteAccess& ) = delete;
        RegistryWriteAccess& operator=( const RegistryWriteAccess& )
            = delete;

        Registry* operator->() { return &m_registry; }
    };

    RegistryReadAccess readRegistry()
    {
        return { m_mutex, m_registry };
    }
    RegistryWriteAccess writeRegistry()
    {
        return { m_mutex, m_registry };
    }
};
} // namespace mega::service

#include "service/ptr.ipp"
