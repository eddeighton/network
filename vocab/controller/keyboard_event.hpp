
#pragma once

#include "common/serialisation.hpp"

#include <string>

namespace mega::controller
{

struct KeyboardEvent
{
    std::string key;
    bool        down;

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

} // namespace mega::controller
