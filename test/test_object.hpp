
#pragma once

#include "test/test.interface.hpp"
#include "test/service/test.proxy.hxx"

#include "service/network.hpp"

#include "common/assert_verify.hpp"

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
};
}


