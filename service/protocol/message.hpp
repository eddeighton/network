

#pragma once

#include "service/protocol/buffer.hpp"
#include "service/protocol/header.hpp"
#include "service/sender.hpp"

#include "common/disable_special_members.hpp"

#include <functional>
#include <variant>
#include <cstdint>

namespace mega::service
{
    using Functor = std::function< void() >;

    // Messages that are dispatched internally
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
        Functor m_functor;
    };

    struct InterProcessResponse 
    {
        Header m_header;
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

    // Protocol for messages sent over the wire
    enum class MessageType : std::uint32_t
    {
        eRegistry   = 0,
        eDisconnect = 1,
        eEnrole     = 2,
        eRequest    = 3,
        eResponse   = 4,
        TOTAL_MESSAGES
    };
}

