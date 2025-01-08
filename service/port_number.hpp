

#pragma once

#include <cstdint>
#include <string>
#include <sstream>
#include <iomanip>

namespace mega::service
{
struct PortNumber
{
    inline explicit operator std::int16_t() const { return value; }

    inline std::string str() const
    {
        std::ostringstream os;
        os << value;
        return std::to_string( value );
    }

    std::int16_t value;
};

inline std::ostream& operator<<( std::ostream&     os,
                                 const PortNumber& portNumber )
{
    return os << std::setw( 4 ) << std::setfill( '0' )
              << portNumber.value;
}
inline std::istream& operator>>( std::istream& is,
                                 PortNumber&   portNumber )
{
    return is >> portNumber.value;
}
} // namespace mega::service
