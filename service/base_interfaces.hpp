
#pragma once

namespace mega::service
{
    struct Factory
    {
        virtual ~Factory() = 0;
    };
     inline Factory::~Factory() = default;

    struct Interface
    {
        virtual ~Interface() = 0;
    };
     inline Interface::~Interface() = default;
}

