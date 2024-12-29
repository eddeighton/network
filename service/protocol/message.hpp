

#pragma once

#include "service/protocol/buffer.hpp"

#include <functional>
#include <variant>

namespace mega::service
{
    using Functor = std::function< void() >;

    struct InProcessRequest
    {
        Functor m_functor;
    };

    struct InProcessResponse 
    {
        Functor m_functor;
    };

    struct InterProcessRequest
    {
        PacketBuffer m_buffer;
    };

    struct InterProcessResponse 
    {
        PacketBuffer m_buffer;
    };

    struct Other
    {
    };

    using Message = std::variant
    <
        InProcessRequest,
        InProcessResponse,
        InterProcessRequest,
        InterProcessResponse,
        Other
    >;
}

