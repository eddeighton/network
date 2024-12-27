
#pragma once

#include "test/service/testfactory.proxy.hxx"
#include "test/service/test.proxy.hxx"

#include "service/registration.hpp"
#include "service/rtti.hpp"
#include "service/ptr.hpp"
#include "service/base_interfaces.hpp"

#include "vocab/service/mpo.hpp"
#include "vocab/service/mptf.hpp"

#include <unordered_map>
#include <vector>
#include <memory>
#include <variant>

namespace mega::service
{
    class Registry
    {
        MP m_mp;

        using ObjectVector = std::vector< Interface* >;
        ObjectVector m_objects;

        using ProxyVariant = std::variant
        <
            std::unique_ptr< test::TestFactory_InProcess >,
            std::unique_ptr< test::TestFactory_InterProcess >
        >;
        using ProxyVariantVector = std::vector< ProxyVariant >;

        ProxyVariantVector m_proxies;

        // using MPOInterface = std::pair< MPO, int >;
        // using ProxyMap = std::unordered_map< MPOInterface, std::InterfaceVariant >;


    public:
        inline Registry( MP machineProcess )
            : m_mp( machineProcess )
        {

        }

        Registry(Registry&) = delete;
        Registry& operator=(Registry&) = delete;

        Registry(Registry&&) = delete;
        Registry& operator=(Registry&&) = delete;

        // using RegistrationMap = std::unordered_map< MPO, RTTI >; 
        inline MPO createInProcessProxy( Interface& object, LogicalThread& logicalThread )
        {
            // assign MPO and MPTF
            VERIFY_RTE_MSG(
                 m_objects.size() < std::numeric_limits< ObjectID::ValueType >::max(),
                 "No remaining ObjectIDs available" );

            const ObjectID objectID{ static_cast< ObjectID::ValueType >( m_objects.size() ) };

            const MPO mpo( m_mp, objectID );

            RTTI rtti;

            if( auto p = dynamic_cast< mega::test::TestFactory* >( &object ) )
            {
                std::unique_ptr< test::TestFactory_InProcess >
                    pProxy = std::make_unique< test::TestFactory_InProcess >(
                        p, logicalThread, rtti );
                m_proxies.push_back( std::move( pProxy ) );
            }

            m_objects.push_back(&object);

            return mpo;
        }

        inline void createInterProcessProxy()
        {

        }
    };
}

