
#pragma once

#include "service/interface/test.interface.hpp"
#include "service/gen/testfactory.proxy.hxx"

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
    service::MPTFO m_mptfo;
    service::Ptr< TestFactory > m_pProxy;

    using TestPtr    = std::unique_ptr< OTest >;
    using TestVector = std::vector< TestPtr >;
    TestVector m_tests;
public:
    OTestFactory(service::Network& network)
    :   m_network( network )
    {
        auto& reg = network.writeRegistry().get();
        m_mptfo = reg.createInProcessProxy(service::LogicalThread::get().getMPTF(), *this);
        m_pProxy = reg.one< TestFactory >(m_mptfo);
    }

    const service::MPTFO getMPTFO() const { return m_mptfo; }
    service::Ptr< TestFactory > getPtr() const { return m_pProxy; }

    service::Ptr<Test> create_test() override
    {
        std::cout << "create_test called in OTestFactory" << std::endl;
        
        TestPtr pTest = std::make_unique< OTest >();
        auto& reg = m_network.writeRegistry().get();
        const auto mpo = reg.createInProcessProxy(m_mptfo.getMPTF(), *pTest);
        m_tests.push_back( std::move( pTest ) );
        return reg.one< Test >( mpo );
    }
};

}

