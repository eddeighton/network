
#pragma once

#include "service/protocol/buffer.hpp"

#include <boost/system/error_code.hpp>

namespace mega::service
{

class Sender
{
public:
    virtual ~Sender() = 0;
    virtual boost::system::error_code send( const PacketBuffer& msg ) = 0;
};
inline Sender::~Sender() = default;

}

