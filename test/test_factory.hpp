
#pragma once

#include "test/test_object.hpp"
#include "test/service/testfactory.proxy.hxx"

#include "service/rtti.hpp"
#include "service/logical_thread.hpp"
#include "service/network.hpp"
#include "service/registry.hpp"
#include "service/ptr.hpp"

#include "common/assert_verify.hpp"

namespace mega::test
{

class OTestFactory : public TestFactory
{
    service::Network& m_network;
    service::MPO m_mpo;
    service::Ptr< TestFactory > m_pProxy;
public:
    OTestFactory(service::Network& network)
    :   m_network( network )
    {
        auto& reg = network.writeRegistry().get();

        m_mpo = reg.createInProcessProxy(*this);

        auto factories = reg.get< TestFactory >( m_mpo );
        VERIFY_RTE(factories.size()==1);
        m_pProxy = factories.front();
    }

    const service::MPO getMPO() const { return m_mpo; }
    service::Ptr< TestFactory > getPtr() const { return m_pProxy; }

    service::Ptr<Test> create_test() override
    {
        return {};
    }
};

void threadRoutine(service::Network& network)
{
    using namespace mega::service;
    {
        network.writeRegistry().get().registerFiber();
    }
    OTestFactory testFactory(network);

    Ptr< TestFactory > pFactory = testFactory.getPtr();
    std::cout << "Created TestFactory: " << testFactory.getMPO() << std::endl;

    boost::fibers::fiber test( [&]()
    {
        {
            network.writeRegistry().get().registerFiber();
        }
        Ptr< Test > pTest = pFactory->create_test();
        std::cout << "Created test object" << std::endl;
    });

    // std::cout << pTest->test1() << std::endl;

    test.join();
}

}

