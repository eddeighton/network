
#pragma once

#include "test/service/testfactory.proxy.hxx"
#include "test/service/test.proxy.hxx"

#include "service/registration.hpp"
#include "service/logical_thread.hpp"
#include "service/rtti.hpp"
#include "service/ptr.hpp"
#include "service/base_interfaces.hpp"

#include "vocab/service/mpo.hpp"
#include "vocab/service/mptf.hpp"
#include "vocab/service/interface_type_name.hpp"

#include "common/disable_special_members.hpp"

#include <boost/type_index.hpp>

#include <map>
#include <vector>
#include <memory>
#include <variant>

namespace mega::service
{
    template< typename InterfaceType >
    InterfaceTypeName getInterfaceTypeName();

    template<>
    inline InterfaceTypeName getInterfaceTypeName< mega::test::TestFactory >()
    {
        using namespace std::string_literals;
        return "mega::test::TestFactory"s;
    } 

    template<>
    inline InterfaceTypeName getInterfaceTypeName< mega::test::Test >()
    {
        using namespace std::string_literals;
        return "mega::test::Test"s;
    }
    
    class Registry : public Common::DisableCopy, Common::DisableMove
    {
        const MP m_mp;

        using ObjectVector = std::vector< Interface* >;
        ObjectVector m_objects;

        using ProxyVariant = std::variant
        <
            std::unique_ptr< test::TestFactory_InProcess >,
            std::unique_ptr< test::TestFactory_InterProcess >,
            std::unique_ptr< test::Test_InProcess >,
            std::unique_ptr< test::Test_InterProcess >
        >;
        using ProxyVariantVector = std::vector< ProxyVariant >;

        ProxyVariantVector m_proxies;

        using MPOInterface = std::pair< MPO, InterfaceTypeName >;
        using MPOInterfaceMap = std::map< MPOInterface, Interface* >;

        MPOInterfaceMap m_mpoInterfaceMap;

        boost::fibers::fiber_specific_ptr< LogicalThread > fiber_local_storage;
    public:
        inline Registry( MP machineProcess )
            : m_mp( machineProcess )
        {
        }

        MP getMP() const { return m_mp; }

        inline MPO createInProcessProxy(Interface& object)
        {
            VERIFY_RTE_MSG(
                 m_objects.size() < std::numeric_limits< ObjectID::ValueType >::max(),
                 "No remaining ObjectIDs available" );

            const ObjectID objectID{static_cast< ObjectID::ValueType >( m_objects.size() )};

            const MPO mpo(m_mp, objectID);

            RTTI rtti;

            auto& logicalThread = LogicalThread::get();

            if( auto p1 = dynamic_cast< mega::test::TestFactory* >( &object ) )
            {
                std::unique_ptr< test::TestFactory_InProcess >
                    pProxy = std::make_unique< test::TestFactory_InProcess >(
                        p1, logicalThread, rtti );
                const auto interfaceTypeName = getInterfaceTypeName< mega::test::TestFactory >();
                m_mpoInterfaceMap.insert(
                    std::make_pair( MPOInterface{ mpo, interfaceTypeName }, pProxy.get() ) );
                m_proxies.push_back( std::move( pProxy ) );
            }
            if( auto p2 = dynamic_cast< mega::test::Test* >( &object ) )
            {
                std::unique_ptr< test::Test_InProcess >
                    pProxy = std::make_unique< test::Test_InProcess >(
                        p2, logicalThread, rtti );
                const auto interfaceTypeName = getInterfaceTypeName< mega::test::Test >();
                m_mpoInterfaceMap.insert(
                    std::make_pair( MPOInterface{ mpo, interfaceTypeName }, pProxy.get() ) );
                m_proxies.push_back( std::move( pProxy ) );
            }

            m_objects.push_back(&object);
            return mpo;
        }

        inline void createInterProcessProxy()
        {

        }

        template< typename T >
        std::vector< Ptr< T > > get(MPO mpo) const
        {
            const InterfaceTypeName interfaceTypeName
                = getInterfaceTypeName< T >();

            const auto key = MPOInterface(mpo, interfaceTypeName);

            const auto iLower = m_mpoInterfaceMap.lower_bound( key );
            const auto iUpper = m_mpoInterfaceMap.upper_bound( key );

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
        Ptr< T > one(MPO mpo) const
        {
            auto r = get<T>(mpo);
            VERIFY_RTE_MSG(r.size()==1,
                "Non-singular result finding type: " <<
                boost::typeindex::type_id<T>().pretty_name());
            return r.front();
        }
    };
}

