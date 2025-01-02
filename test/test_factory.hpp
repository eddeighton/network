
#pragma once

#include "test/test_object.hpp"
#include "service/interface/test.interface.hpp"

#include "service/rtti.hpp"
#include "service/logical_thread.hpp"
#include "service/registry.hpp"
#include "service/ptr.hpp"

#include "common/assert_verify.hpp"

namespace mega::test
{

class OTestFactory : public TestFactory
{
    service::MPTFO m_mptfo;
    service::Ptr< TestFactory > m_pProxy;

    using TestPtr    = std::unique_ptr< OTest >;
    using TestVector = std::vector< TestPtr >;
    TestVector m_tests;
public:
    OTestFactory()
    {
        auto reg = service::Registry::getWriteAccess();
        m_mptfo = reg->createInProcessProxy(service::LogicalThread::get().getMPTF(), *this);
        m_pProxy = reg->one< TestFactory >(m_mptfo);
    }

    const service::MPTFO getMPTFO() const { return m_mptfo; }
    service::Ptr< TestFactory > getPtr() const { return m_pProxy; }

    service::Ptr<Test> create_test() override
    {
        std::cout << "create_test called in OTestFactory" << std::endl;
        
        TestPtr pTest = std::make_unique< OTest >();
        auto reg = service::Registry::getWriteAccess();
        const auto mpo = reg->createInProcessProxy(m_mptfo.getMPTF(), *pTest);
        m_tests.push_back( std::move( pTest ) );
        return reg->one< Test >( mpo );
    }
};

}

