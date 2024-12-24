
#include "test/test.interface.hpp"

#include <string>
#include <memory>

namespace mega::test
{

class OTest : public Test
{
public:
    virtual std::string test1() 
    {
        using namespace std::string_literals;
        return "Hello World"s;
    }
    virtual std::string test2(int i)
    {
        using namespace std::string_literals;
        return "Hello World"s;
    }
    virtual std::string test3(std::string& str)
    {
        using namespace std::string_literals;
        return "Hello World"s;
    }
    virtual std::string test4(const std::vector<int>& v, Test* pTest)
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


