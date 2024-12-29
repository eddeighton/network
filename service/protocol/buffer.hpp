

#pragma once

#include <cstdint>
#include <vector>

namespace mega::service
{
    using PacketSizeType = std::uint64_t;
    static constexpr auto PacketSizeSize = sizeof(PacketSizeType);
    
    using PacketBuffer = std::vector< char >;
}

