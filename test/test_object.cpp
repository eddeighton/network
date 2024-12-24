
#include "test/test.interface.hpp"

#include <string>
#include <memory>

namespace mega::test
{

class OTest : public Test
{
public:
    std::string test() override
    {
        using namespace std::string_literals;
        return "Hello World"s;
    }
};

class OTestFactory : public TestFactory
{
public:
    std::shared_ptr<Test> create_test() override
    {
        return std::make_shared<OTest>();
    }
};

}


