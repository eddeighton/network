
#pragma once
// Sun Dec 29 05:23:02 UTC 2024

#include "service/base_interfaces.hpp"
#include "service/ptr.hpp"

#include <string>

namespace mega
{
namespace test
{
struct Test : public virtual service::Interface
{
    virtual std::string test1()                             = 0;
    virtual int         test2( int i )                      = 0;
    virtual std::string test3( std::string str )            = 0;
    virtual std::string test4( service::Ptr< Test > pTest ) = 0;
    virtual void shutdown() = 0;
};

struct TestFactory : public virtual mega::service::Factory
{
    virtual service::Ptr< Test > create_test() = 0;
};
} // namespace test
} // namespace mega
