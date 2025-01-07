
#pragma once

#include "service/interface/test.interface.hpp"

#include "common/assert_verify.hpp"

#include <iostream>

namespace mega::test
{
class OTest : public Test
{
public:
    std::string test1() override
    {
        // std::cout << "Hello World inside OTest::test1" << std::endl;
        return "Hello World";
    }

    int test2(int i) override
    {
        return i;
    }

    std::string test3(std::string str) override
    {
        return str;
    }
};
}

