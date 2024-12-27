


#include "test/test_object.hpp"
#include "test/test_factory.hpp"

#include "service/network.hpp"
#include "service/ptr.hpp"

namespace mega::test
{
    void runTestComponent(service::Network& network)
    {
        using namespace service;

        OTestFactory factory( network );

        std::cout << "Created TestFactory: " << factory.getMPO() << std::endl;

        Ptr< TestFactory > pFactory = factory.getPtr();

        Ptr< Test > pTest = pFactory->create_test();

        std::cout << pTest->test1() << std::endl;
    }
}

