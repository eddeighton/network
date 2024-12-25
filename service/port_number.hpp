

#pragma once

#include <cstdint>
#include <string>

namespace mega::service
{
    struct PortNumber
    {
        inline explicit operator std::int16_t() const { return value; }

        inline std::string str() const
        {
            std::ostringstream os;
            os << value;
            return std::to_string(value);
        }

        std::int16_t value;
    };
}

