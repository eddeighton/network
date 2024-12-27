
#pragma once

#include "test/test_object.hpp"
#include "test/service/testfactory.proxy.hxx"

#include "service/rtti.hpp"
#include "service/logical_thread.hpp"

#include "common/assert_verify.hpp"
namespace mega::test
{

class OTestFactory : public TestFactory
{
    static inline std::unique_ptr< OTestFactory > g_pObject;
    service::Network& m_network;
    service::LogicalThread m_logicalThread;
public:
    OTestFactory( service::Network& network )
    :   m_network( network )
    {
    }

    static service::Ptr< TestFactory > create( service::Network& network )
    {
        g_pObject = std::make_unique< OTestFactory >( network );

        const auto rtti = getRTTI( g_pObject.get() );

        //TestFactory_InProcess proxy( g_pObject.get(), rtti, g_pObject->m_logicalThread );

        //network.writeRegistration().get().registerProxy( proxy );

        THROW_TODO;

       // return Ptr{ &proxy };
    }

    service::Ptr<Test> create_test() override
    {
        THROW_TODO;
    }
};


}

