
#pragma once

#include "service/base_interfaces.hpp"

#include <string>
#include <memory>

namespace mega
{
    namespace test
    {
        struct Test : public service::Interface
        {
            virtual std::string test() = 0;
        };
        
        struct TestFactory : public service::Factory
        {
            virtual std::shared_ptr<Test> create_test() = 0;
        };
    }
}

