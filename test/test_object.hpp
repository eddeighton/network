
#pragma once

#include "service/interface/test.interface.hpp"

#include "service/gen/test.proxy.hxx"

#include "common/assert_verify.hpp"

#include <string>
#include <memory>
#include <iostream>

namespace mega::test
{

class OTest : public Test
{
public:
    virtual std::string test1() 
    {
        using namespace std::string_literals;
        std::cout << "Hello World inside OTest::test1"s << std::endl;
        return "Hello World"s;
    }
};
}


