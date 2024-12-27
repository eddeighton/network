

#include <iostream>
#include <variant>
#include <memory>
#include <vector>

struct Base
{
    virtual ~Base() = 0;
};
inline Base::~Base() = default;

struct Test
{
    struct Baz : public Base
    {
        int x = 0;

        Baz()
        {
        }
        Baz(Baz&)=delete;
    };

    struct Foobar : public Base
    {
        int x = 0;
    };

    using BazPtr    = std::unique_ptr< Baz >;
    using FoobarPtr = std::unique_ptr< Foobar >;

    using VarPtr       = std::variant< BazPtr, FoobarPtr >;
    using VarPtrVector = std::vector< VarPtr >;

    void testing()
    {
        VarPtrVector test;

        auto p1 = std::make_unique<Baz>();
        test.push_back( std::move( p1 ) );

        auto p2 = std::make_unique<Foobar>();
        test.push_back( std::move( p2 ) );

        std::cout << "Hello World: " << test.size() << std::endl;
    }

};

int main( int argc, const char* argv[] )
{
    Test test;
    test.testing();
    return 0;
}

