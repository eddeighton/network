

#include "demo.hpp"

#include <iostream>

int main( int argc, const char* argv[] )
{
    try
    {
        retail::Demo demo;
        demo.run();
    }
    catch ( std::exception& ex )
    {
        std::cerr << "Exception: " << ex.what() << std::endl;
        return 1;
    }
    catch ( ... )
    {
        std::cerr << "Unknown exception" << std::endl;
        return 1;
    }
    return 0;
}
