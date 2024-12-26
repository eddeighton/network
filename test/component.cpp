


#include "test/test_object.hpp"

#include "test/test_factory.hpp"

#include "service/network.hpp"

namespace mega::test
{
    void runTestComponent()
    {

        service::Network network;

        service::Ptr< TestFactory > ptr = OTestFactory::create( network );


    }
}

