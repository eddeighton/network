

#pragma once

#include "service/protocol/buffer.hpp"
#include "service/protocol/header.hpp"
#include "service/sender.hpp"

#include "common/disable_special_members.hpp"

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
        Sender* m_responseSender;
        Header m_header;
        PacketBuffer m_buffer;
    };

    struct InterProcessResponse 
    {
        Header m_header;
        PacketBuffer m_buffer;
    };

    struct Other //: public Common::DisableCopy
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

