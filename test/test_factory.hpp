
#pragma once

#include "test/test_object.hpp"
#include "test/service/testfactory.ptr.hxx"

#include "service/rtti.hpp"

namespace mega::test
{

class OTestFactory : public TestFactory
{
public:

    static service::Ptr< TestFactory > create( service::Network& network )
    {
        auto reg = network.writeRegistration();

        std::unique_ptr< OTestFactory > pObject =
            std::make_unique< OTestFactory >();

        const auto rtti = getRTTI( pObject.get() );

        service::Ptr< TestFactory > ptr( pObject.get(), rtti );

        reg.get().register_ptr( ptr );

        return ptr;
    }

    service::Ptr<Test> create_test() override
    {
        
        
        THROW_TODO;
        // return std::make_shared<OTest>();
    }
};


}

