
#pragma once

#include "service/base_interfaces.hpp"
#include "service/ptr.hpp"

#include <string>
#include <memory>
#include <vector>

namespace mega
{
    namespace test
    {
        struct Test : public virtual service::Interface
        {
            virtual std::string test1() = 0;
        };
        
        struct TestFactory : public virtual mega::service::Factory
        {
            virtual service::Ptr<Test> create_test() = 0;
        };

        struct Connectivity : public virtual mega::service::Daemon
        {
            virtual void connect() = 0;
            virtual void disconnect() = 0;
            virtual void shutdown() = 0;
        };

    }
}

