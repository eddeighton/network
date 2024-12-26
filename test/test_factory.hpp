
#pragma once

#include "test/test_object.hpp"
#include "test/service/testfactory.ptr.hxx"

#include "service/rtti.hpp"
#include "service/logical_thread.hpp"

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

        service::Ptr< TestFactory > ptr( g_pObject.get(), rtti, g_pObject->m_logicalThread );

        network.writeRegistration().get().register_ptr( ptr );

        return ptr;
    }

    service::Ptr<Test> create_test() override
    {
        
        
        THROW_TODO;
        // return std::make_shared<OTest>();
    }
};


}

