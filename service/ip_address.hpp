#pragma once

#include <string>
#include <ostream>

namespace mega::service
{
struct IPAddress
{
    std::string value;
};
inline std::ostream& operator<<( std::ostream&    os,
                                 const IPAddress& ipAddress )
{
    return os << ipAddress.value;
}
inline std::istream& operator>>( std::istream& is, IPAddress& isAddress )
{
    return is >> isAddress.value;
}
} // namespace mega::service
