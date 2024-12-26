
#include "test/test.interface.hpp"
#include "test/service/test.proxy.hxx"
#include "test/service/testfactory.proxy.hxx"

#include "service/network.hpp"

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

    static TestFactoryPtr* create( service::Network& network )
    {
        std::unique_ptr< TestFactoryPtr > p;

        

        return p.get();
    }

    std::shared_ptr<Test> create_test() override
    {
          
        
        
        return std::make_shared<OTest>();
    }
};

}


