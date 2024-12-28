
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

    using TestPtr    = std::unique_ptr< OTest >;
    using TestVector = std::vector< TestPtr >;
    TestVector m_tests;
public:
    OTestFactory(service::Network& network)
    :   m_network( network )
    {
        auto& reg = network.writeRegistry().get();
        m_mpo = reg.createInProcessProxy(*this);
        m_pProxy = reg.one< TestFactory >(m_mpo);
    }

    const service::MPO getMPO() const { return m_mpo; }
    service::Ptr< TestFactory > getPtr() const { return m_pProxy; }

    service::Ptr<Test> create_test() override
    {
        std::cout << "create_test called in OTestFactory" << std::endl;
        
        TestPtr pTest = std::make_unique< OTest >();
        auto& reg = m_network.writeRegistry().get();
        const auto mpo = reg.createInProcessProxy(*pTest);
        m_tests.push_back( std::move( pTest ) );
        return reg.one< Test >( mpo );
    }
};

void threadRoutine(service::Network& network)
{
    using namespace mega::service;
    LogicalThread::registerFiber(network.getMP());
    LogicalThread& thread = LogicalThread::get();

    OTestFactory testFactory(network);

    Ptr< TestFactory > pFactory = testFactory.getPtr();
    std::cout << "Created TestFactory: " << testFactory.getMPO() << std::endl;
    Ptr< Test > pTest2 = pFactory->create_test();
    std::cout << "Test returned: " << pTest2->test1() << std::endl;

    boost::fibers::fiber test( [&]()
    {
        LogicalThread::registerFiber(network.getMP());
        try
        {
            Ptr< Test > pTest = pFactory->create_test();
            std::cout << "Test returned: " << pTest->test1() << std::endl;
        }
        catch( std::exception& ex )
        {
            std::cout << "Caught exception: " << ex.what() <<
                " In: " << LogicalThread::get().getMPTF() << std::endl;
        }

        thread.stop();
    });

    // LogicalThread::get().receive();
    // std::cout << pTest->test1() << std::endl;
    LogicalThread::get().runMessageLoop();
    
    test.join();
}

}

