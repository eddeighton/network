
#pragma once

#include "service/registration.hpp"
#include "service/logical_thread.hpp"
#include "service/rtti.hpp"
#include "service/ptr.hpp"
#include "service/base_interfaces.hpp"

#include "vocab/service/mptfo.hpp"
#include "vocab/service/mptf.hpp"
#include "vocab/service/interface_type_name.hpp"

#include "service/gen/registry.hxx"

#include "common/disable_special_members.hpp"

#include <map>
#include <vector>
#include <memory>
#include <variant>
#include <unordered_map>
#include <mutex>
#include <shared_mutex>

namespace mega::service
{
    class Registry : public Common::DisableCopy, Common::DisableMove
    {
    public:
        struct ObjectInfo
        {
            Interface* pObject;
            LogicalThread& logicalThread;
        };
    private:
        using Objects        = std::unordered_map< MPTFO, Interface*,     MPTFO::Hash >;
        using LogicalThreads = std::unordered_map< MPTF,  LogicalThread*, MPTF::Hash >;

        Objects             m_objects;
        LogicalThreads      m_logicalThreads;
        ProxyVariantVector  m_proxies;
        InterfaceMPTFOMap   m_interfaceMPTFOMap;

    public:
        inline Registration getRegistration() const
        {
            Registration registration;
            std::set<Registration::Registrant> uniqueRegistrants;
            for(const auto& proxy : m_proxies)
            {
                std::visit([&uniqueRegistrants](const auto& pProxyUniquePtr)
                {
                    uniqueRegistrants.insert
                    (
                        Registration::Registrant
                        {
                            pProxyUniquePtr->m_mptfo,
                            pProxyUniquePtr->m_rtti
                        }
                    );
                }, proxy);
            }
            std::copy(uniqueRegistrants.begin(), uniqueRegistrants.end(),
                std::back_inserter(registration.m_registrants));
            return registration;
        }

        void update(Sender& sender, const Registration& registration )
        {
            service::update(sender, registration, m_proxies, m_interfaceMPTFOMap);
        }

        inline MPTFO createInProcessProxy(MPTF mptf, Interface& object)
        {
            VERIFY_RTE_MSG(
                 m_objects.size() < std::numeric_limits< ObjectID::ValueType >::max(),
                 "No remaining ObjectIDs available" );

            const ObjectID objectID{static_cast< ObjectID::ValueType >(m_objects.size())};
            const MPTFO mptfo(mptf, objectID);
            registerInProcessProxy(object, mptfo, m_proxies, m_interfaceMPTFOMap);
            m_objects.insert(std::make_pair(mptfo, &object));
            return mptfo;
        }

        LogicalThread& getLogicalThread(MPTF mptf) const
        {
            auto iFind = m_logicalThreads.find(mptf);
            VERIFY_RTE_MSG(iFind != m_logicalThreads.end(),
                "Failed to locate logical thread: " << mptf);
            return *iFind->second;
        }

        void registerLogicalThread(MPTF mptf, LogicalThread* pLogicalThread)
        {
            auto ib = m_logicalThreads.insert(std::make_pair(mptf, pLogicalThread));
            VERIFY_RTE_MSG( ib.second,
                "Failed to register logical thread.  Duplicate mptf found: " << mptf );
            auto iFind = m_logicalThreads.find(mptf);
            VERIFY_RTE_MSG(iFind != m_logicalThreads.end(),
                "Failed to locate logical thread straight away: " << mptf);
            // std::cout << "Registry registered logical thread: " << mptf << std::endl;
        }

        template< typename T >
        std::vector< std::shared_ptr< Proxy< T > > > get(MPTFO mptfo) const
        {
            const InterfaceTypeName interfaceTypeName
                = getInterfaceTypeName< T >();

            const auto key = InterfaceMPTFO(interfaceTypeName, mptfo);

            const auto iLower = m_interfaceMPTFOMap.lower_bound( key );
            const auto iUpper = m_interfaceMPTFOMap.upper_bound( key );

            std::vector< std::shared_ptr< Proxy< T > > > result;
            for( auto i = iLower; i != iUpper; ++i )
            {
                if( auto p = std::dynamic_pointer_cast< Proxy<T> >( i->second ) )
                {
                    result.push_back( p );
                }
            }
            return result;
        }

        template< typename T >
        std::shared_ptr< Proxy< T > > one(MPTFO mptfo) const
        {
            auto r = get<T>(mptfo);
            VERIFY_RTE_MSG(r.size()!=0,
                "Found no matches for mptfo: " << mptfo << " and interface: " <<
                boost::typeindex::type_id<T>().pretty_name());
            VERIFY_RTE_MSG(r.size()==1,
                "Non-singular result for mptfo: " << mptfo  << " and interface: " <<
                boost::typeindex::type_id<T>().pretty_name());
            return r.front();
        }

        template< typename T >
        std::vector< std::shared_ptr< Proxy< T > > > get(MP mp) const
        {
            const InterfaceTypeName interfaceTypeName
                = getInterfaceTypeName< T >();

            const auto key = InterfaceMPTFO(interfaceTypeName, MPTFO(mp, ThreadID{}, FiberID{}, ObjectID{}));

            const auto iLower = m_interfaceMPTFOMap.lower_bound( key );

            std::vector< std::shared_ptr< Proxy< T > > > result;
            for( auto i = iLower; i != m_interfaceMPTFOMap.end(); ++i )
            {
                if( i->first.second.getMP() != mp )
                {
                    break;
                }
                if( auto p = std::dynamic_pointer_cast< Proxy<T> >( i->second ) )
                {
                    result.push_back( p );
                }
            }
            return result;
        }

        template< typename T >
        std::shared_ptr< Proxy< T > > one(MP mp) const
        {
            auto r = get<T>(mp);
            VERIFY_RTE_MSG(r.size()!=0,
                "Could not find instance of proxy type: " <<
                boost::typeindex::type_id<T>().pretty_name());
            VERIFY_RTE_MSG(r.size()==1,
                "Non-singular result finding type: " <<
                boost::typeindex::type_id<T>().pretty_name());
            return r.front();
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

        static RegistryReadAccess getReadAccess();
        static RegistryWriteAccess getWriteAccess();
    };

}

#include "service/ptr.ipp"

