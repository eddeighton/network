


#include "test/test_object.hpp"
#include "test/test_factory.hpp"

#include "service/network.hpp"

namespace mega::test
{
    void runTestComponent(service::Network& network)
    {
        using namespace service;

        // Ptr< TestFactory > pFactory = 
            OTestFactory::create( network );

        // Ptr< Test > pTest = pFactory->create_test();

        // std::cout << pTest->test1() << std::endl;
    }
}

