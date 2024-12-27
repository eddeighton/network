
#pragma once

#include "test/test_object.hpp"
#include "test/service/testfactory.proxy.hxx"

#include "service/rtti.hpp"
#include "service/logical_thread.hpp"
#include "service/network.hpp"
#include "service/registry.hpp"

#include "common/assert_verify.hpp"

namespace mega::test
{

class OTestFactory : public TestFactory
{
    service::Network& m_network;
    service::LogicalThread m_logicalThread;
public:
    OTestFactory( service::Network& network )
    :   m_network( network )
    {
    }

    static service::Ptr< TestFactory > create( service::Network& network )
    {
        static thread_local auto pObject = std::make_unique< OTestFactory >( network );
        auto& lt = pObject->m_logicalThread;

        auto& reg = network.writeRegistry().get();

        const service::MPO mpo = 
            reg.createInProcessProxy(*pObject, lt);

        // THROW_TODO; 

       // return Ptr{ &proxy };
        return {};
    }

    service::Ptr<Test> create_test() override
    {
        THROW_TODO;
    }
};


}

