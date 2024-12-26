
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
            virtual std::string test2(int i) = 0;
            virtual std::string test3(std::string& str) = 0;
            virtual std::string test4(const std::vector<int>& v, Test* pTest) = 0;
        };

        struct Other : public virtual service::Interface
        {
            virtual void another_test( int x ) = 0;
            virtual void foobar( bool x ) = 0;
        };
        
        struct TestFactory : public virtual mega::service::Factory
        {
            virtual service::Ptr<Test> create_test() = 0;
        };
    }
}

