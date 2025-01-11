
#pragma once

#include "common/serialisation.hpp"

#include <string>

namespace mega::controller
{

struct KeyboardEvent
{
    std::string key;
    bool        down;

    inline bool operator<( const KeyboardEvent& cmp ) const
    {
        return std::tie( key, down ) < std::tie( cmp.key, cmp.down );
    }
    inline bool operator==( const KeyboardEvent& cmp ) const
    {
        return std::tie( key, down ) == std::tie( cmp.key, cmp.down );
    }

    template < class Archive >
    inline void serialize( Archive& archive, const unsigned int )
    {
        if constexpr( boost::serialization::IsXMLArchive<
                          Archive >::value )
        {
            archive& boost::serialization::make_nvp( "keycode", key );
            archive& boost::serialization::make_nvp( "down", down );
        }
        else
        {
            archive & key;
            archive & down;
        }
    }
};

inline std::ostream& operator<<( std::ostream& os, const KeyboardEvent& ke )
{
    return os << ke.key << '_' << ( ke.down ? "DOWN" : "UP" );
}

} // namespace mega::controller
