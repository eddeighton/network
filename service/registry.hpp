
#pragma once

#include "service/gen/testfactory.proxy.hxx"
#include "service/gen/test.proxy.hxx"
#include "service/gen/daemon.proxy.hxx"

#include "service/registration.hpp"
#include "service/logical_thread.hpp"
#include "service/rtti.hpp"
#include "service/ptr.hpp"
#include "service/base_interfaces.hpp"

#include "vocab/service/mptfo.hpp"
#include "vocab/service/mptf.hpp"
#include "vocab/service/interface_type_name.hpp"

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
        using ObjectMap = std::unordered_map< MPTFO, ObjectInfo, MPTFO::Hash >;
        ObjectMap m_objects;

        using ProxyVariant = std::variant
        <
            std::unique_ptr< test::TestFactory_InProcess >,
            std::unique_ptr< test::TestFactory_InterProcess >,
            std::unique_ptr< test::Test_InProcess >,
            std::unique_ptr< test::Test_InterProcess >,
            std::unique_ptr< test::Connectivity_InProccess >,
            std::unique_ptr< test::Connectivity_InterProcess >
        >;
        using ProxyVariantVector = std::vector< ProxyVariant >;

        ProxyVariantVector m_proxies;

        using InterfaceMPTFO    = std::pair< InterfaceTypeName, MPTFO >;
        using InterfaceMPTFOMap = std::map< InterfaceMPTFO, Interface* >;

        InterfaceMPTFOMap m_interfaceMPTFOMap;

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

        template< typename TInterface, typename TProxy >
        void registerIfInterfaceIP(Sender& sender, const Registration::Registrant& registrant)
        {
            const auto interfaceTypeName = getInterfaceTypeName< TInterface >();
            if(registrant.m_rtti.contains(interfaceTypeName))
            {
                const auto key = InterfaceMPTFO{interfaceTypeName, registrant.m_mptfo};
                if(m_interfaceMPTFOMap.find(key) == m_interfaceMPTFOMap.end())
                {
                    auto pProxy = std::make_unique<TProxy>(sender, registrant.m_mptfo, registrant.m_rtti);
                    m_interfaceMPTFOMap.insert(std::make_pair(key, pProxy.get()));
                    m_proxies.push_back(std::move(pProxy));
                }
            }
        }

        void update(Sender& sender, const Registration& registration )
        {
            for( const auto& registrant : registration.m_registrants )
            {
                registerIfInterfaceIP< test::TestFactory,  test::TestFactory_InterProcess   >(sender, registrant);
                registerIfInterfaceIP< test::Test,         test::Test_InterProcess          >(sender, registrant);
                registerIfInterfaceIP< test::Connectivity, test::Connectivity_InterProcess  >(sender, registrant);
            }
        }

        template< typename TInterface >
        void conditionallyAddRTTI(Interface* pInterface, RTTI& rtti)
        {
            if(dynamic_cast< TInterface* >( pInterface ))
            {
                rtti.m_interfaces.push_back(getInterfaceTypeName< TInterface >());
            }
        }

        template< typename TInterface, typename TProxy >
        void registerIfInterface(Interface* pInterface, MPTFO mptfo, const RTTI& rtti)
        {
            if(auto p = dynamic_cast< TInterface* >( pInterface ))
            {
                auto pProxy = std::make_unique< TProxy >(p, LogicalThread::get(), mptfo, rtti);
                const auto interfaceTypeName = getInterfaceTypeName< TInterface >();
                m_interfaceMPTFOMap.insert(
                    std::make_pair( InterfaceMPTFO{ interfaceTypeName, mptfo }, pProxy.get() ) );
                m_proxies.push_back( std::move( pProxy ) );
            }
        }

        inline MPTFO createInProcessProxy(MPTF mptf, Interface& object)
        {
            VERIFY_RTE_MSG(
                 m_objects.size() < std::numeric_limits< ObjectID::ValueType >::max(),
                 "No remaining ObjectIDs available" );

            const ObjectID objectID{static_cast< ObjectID::ValueType >(m_objects.size())};

            const MPTFO mptfo(mptf, objectID);

            RTTI rtti;
            conditionallyAddRTTI< test::TestFactory  >( &object, rtti );
            conditionallyAddRTTI< test::Test         >( &object, rtti );
            conditionallyAddRTTI< test::Connectivity >( &object, rtti );

            registerIfInterface< test::TestFactory,  test::TestFactory_InProcess   >( &object, mptfo, rtti );
            registerIfInterface< test::Test,         test::Test_InProcess          >( &object, mptfo, rtti );
            registerIfInterface< test::Connectivity, test::Connectivity_InProccess >( &object, mptfo, rtti );

            m_objects.insert(
                std::make_pair(
                    mptfo,
                    ObjectInfo
                    {
                        &object,
                        LogicalThread::get()
                    })
                );
            return mptfo;
        }

        const ObjectInfo& getObjectInfo(MPTFO mptfo) const
        {
            auto iFind = m_objects.find(mptfo);
            VERIFY_RTE_MSG(iFind!=m_objects.end(),
                "Failed to locate object: " << mptfo);
            return iFind->second;
        }

        template< typename T >
        std::vector< Ptr< T > > get(MPTFO mptfo) const
        {
            const InterfaceTypeName interfaceTypeName
                = getInterfaceTypeName< T >();

            const auto key = InterfaceMPTFO(interfaceTypeName, mptfo);

            const auto iLower = m_interfaceMPTFOMap.lower_bound( key );
            const auto iUpper = m_interfaceMPTFOMap.upper_bound( key );

            std::vector< Ptr< T > > result;
            for( auto i = iLower; i != iUpper; ++i )
            {
                auto p = dynamic_cast< Proxy<T>* >( i->second );
                VERIFY_RTE(p);
                result.push_back( Ptr< T >( p ) );
            }
            return result;
        }

        template< typename T >
        Ptr< T > one(MPTFO mptfo) const
        {
            auto r = get<T>(mptfo);
            VERIFY_RTE_MSG(r.size()==1,
                "Non-singular result finding type: " <<
                boost::typeindex::type_id<T>().pretty_name());
            return r.front();
        }

        template< typename T >
        std::vector< Ptr< T > > get(MP mp) const
        {
            const InterfaceTypeName interfaceTypeName
                = getInterfaceTypeName< T >();

            const auto key = InterfaceMPTFO(interfaceTypeName, MPTFO(mp, ThreadID{}, FiberID{}, ObjectID{}));

            const auto iLower = m_interfaceMPTFOMap.lower_bound( key );

            std::vector< Ptr< T > > result;
            for( auto i = iLower; i != m_interfaceMPTFOMap.end(); ++i )
            {
                if( i->first.second.getMP() != mp )
                {
                    break;
                }
                auto p = dynamic_cast< Proxy<T>* >( i->second );
                VERIFY_RTE(p);
                result.push_back( Ptr< T >( p ) );
            }
            return result;
        }

        template< typename T >
        Ptr< T > one(MP mp) const
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

        static inline RegistryReadAccess getReadAccess();
        static inline RegistryWriteAccess getWriteAccess();
    };

    namespace detail
    {
        // Global registry singleton with reader writer lock
        static inline std::shared_mutex g_mutex;
        static inline Registry g_registry;
    }

    inline Registry::RegistryReadAccess Registry::getReadAccess()
    {
        return {detail::g_mutex, detail::g_registry};
    }
    inline Registry::RegistryWriteAccess Registry::getWriteAccess()
    {
        return {detail::g_mutex, detail::g_registry};
    }



}

