
#pragma once

#include "service/interface/test.interface.hpp"

#include "common/assert_verify.hpp"

#include <iostream>

namespace mega::test
{
class OTest : public Test
{
public:
    virtual std::string test1() 
    {
        std::cout << "Hello World inside OTest::test1" << std::endl;
        return "Hello World";
    }
};
}

