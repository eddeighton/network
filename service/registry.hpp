
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
#include <functional>

namespace mega::service
{
    class Access;

    class Registry : public Common::DisableCopy, Common::DisableMove
    {
    public:
        // struct ObjectInfo
        // {
        //     Interface* pObject;
        //     LogicalThread& logicalThread;
        // };
        using CreationCallback = std::function< void(Registration) >;
    private:
        using Objects        = std::unordered_map< MPTFO, Interface*,     MPTFO::Hash >;

        Access&                           m_access;
        Objects                           m_objects;
        ProxyVariantVector                m_proxies;
        InterfaceMPTFOMap                 m_interfaceMPTFOMap;
        std::optional< CreationCallback > m_creationCallback;

    public:
        Registry( Access& access )
        :   m_access( access )
        {

        }

        inline Registration getRegistration() const
        {
            Registration registration;
            for(const auto& proxy : m_proxies)
            {
                std::visit([&registration](const auto& pProxyUniquePtr)
                {
                    registration.m_registrants.insert
                    (
                        Registration::Registrant
                        {
                            pProxyUniquePtr->m_mptfo,
                            pProxyUniquePtr->m_rtti
                        }
                    );
                }, proxy);
            }
            return registration;
        }

        inline void setCreationCallback( CreationCallback creationCallback )
        {
            m_creationCallback = creationCallback;
        }

        inline void update(Sender& sender, const Registration& registration )
        {
            service::update(m_access, sender, registration, m_proxies, m_interfaceMPTFOMap);
        }

        inline void disconnected(MP mp)
        {
            std::erase_if( m_objects,
                    [mp](const auto& p)
                    {
                        return p.first.getMP() == mp;
                    });

            std::erase_if( m_interfaceMPTFOMap,
                    [mp](const auto& p)
                    {
                        return p.first.second.getMP() == mp;
                    });

            ProxyVariantVector temp;
            std::swap( temp, m_proxies );
            for(const auto& proxy : temp)
            {
                std::visit([&proxies = m_proxies, mp](auto& pProxyUniquePtr)
                {
                    if( pProxyUniquePtr->m_mptfo.getMP() != mp )
                    {
                        proxies.push_back( std::move(pProxyUniquePtr) );
                    }
                }, proxy);
            }
        }

        inline MPTFO createInProcessProxy(MPTF mptf, Interface& object)
        {
            VERIFY_RTE_MSG(
                 m_objects.size() < std::numeric_limits< ObjectID::ValueType >::max(),
                 "No remaining ObjectIDs available" );

            const ObjectID objectID{static_cast< ObjectID::ValueType >(m_objects.size())};
            const MPTFO mptfo(mptf, objectID);
            registerInProcessProxy(m_access, object, mptfo, m_proxies, m_interfaceMPTFOMap);
            m_objects.insert(std::make_pair(mptfo, &object));
            VERIFY_RTE_MSG( m_creationCallback.has_value(),
                "No creation callback set" );
            (*m_creationCallback)(getRegistration());
            return mptfo;
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
    };

}


