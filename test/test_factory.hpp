
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
    service::LogicalThread m_logicalThread;
    service::MPO m_mpo;
    service::Ptr< TestFactory > m_pProxy;
public:
    OTestFactory( service::Network& network )
    :   m_network( network )
    {
        auto& reg = network.writeRegistry().get();

        m_mpo = reg.createInProcessProxy(*this, m_logicalThread);

        auto factories = reg.get< TestFactory >( m_mpo );
        VERIFY_RTE(factories.size()==1);
        m_pProxy = factories.front();
    }

    const service::MPO getMPO() const { return m_mpo; }
    service::Ptr< TestFactory > getPtr() const { return m_pProxy; }

    service::Ptr<Test> create_test() override
    {
        THROW_TODO;
    }
};


}

