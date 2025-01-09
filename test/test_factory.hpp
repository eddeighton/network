
#pragma once

#include "test/test_object.hpp"
#include "service/interface/test.interface.hpp"

#include "service/access.hpp"
#include "service/logical_thread.hpp"
#include "service/registry.hpp"
#include "service/ptr.hpp"

#include "common/log.hpp"

namespace mega::test
{

class OTestFactory : public TestFactory
{
    service::Access& m_access;
    service::MPTFO m_mptfo;
    service::Ptr< TestFactory > m_pProxy;

    using TestPtr    = std::unique_ptr< OTest >;
    using TestVector = std::vector< TestPtr >;
    TestVector m_tests;
public:
    OTestFactory(service::Access& access)
    :   m_access(access)
    {
        auto reg = m_access.writeRegistry();
        m_mptfo = reg->createInProcessProxy(service::LogicalThread::get().getMPTF(), *this);
        m_pProxy = reg->one< TestFactory >(m_mptfo);
    }

    const service::MPTFO getMPTFO() const { return m_mptfo; }
    service::Ptr< TestFactory > getPtr() const { return m_pProxy; }

    service::Ptr<Test> create_test() override
    {
        TestPtr pTest = std::make_unique< OTest >();
        auto reg = m_access.writeRegistry();
        const service::MPTFO mptfo = reg->createInProcessProxy(m_mptfo.getMPTF(), *pTest);
        LOG( "create_test called in OTestFactory created: " << mptfo );
        m_tests.push_back( std::move( pTest ) );
        return reg->one< Test >( mptfo );
    }
};

}

