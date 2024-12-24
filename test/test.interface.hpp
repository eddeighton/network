
#include "service/base_interfaces.hpp"

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
        
        struct TestFactory : public virtual mega::service::Factory
        {
            virtual std::shared_ptr<Test> create_test() = 0;
        };
    }
}

