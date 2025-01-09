
#pragma once

#include "service/interface/test.interface.hpp"

#include "common/log.hpp"

namespace mega::test
{
class OTest : public Test
{
public:
    inline std::string test1() override
    {
        return "Hello World";
    }

    inline int test2(int i) override
    {
        return i;
    }

    inline std::string test3(std::string str) override
    {
        return str;
    }

    inline std::string test4( service::Ptr< Test > pTest ) override
    {
        if( pTest )
        {
            auto result = pTest->test1();
            LOG( "test4 calling pTest->test1() and got: " << result );
            return result;
        }
        return "";
    }
};
}

