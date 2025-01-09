
#pragma once

#include "common/serialisation.hpp"

namespace mega::controller
{

struct KeyboardEvent
{
    int value;

    template < class Archive >
    inline void serialize( Archive& archive, const unsigned int )
    {
        if constexpr( boost::serialization::IsXMLArchive<
                          Archive >::value )
        {
            archive& boost::serialization::make_nvp(
                "keyboard_event", value );
        }
        else
        {
            archive & value;
        }
    }
};

} // namespace mega::service
