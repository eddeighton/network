
#pragma once

#include "service/enrole.hpp"
#include "service/registration.hpp"

#include "service/protocol/message.hpp"
#include "service/protocol/serialization.hpp"

namespace mega::service
{
    inline void sendEnrole( const Enrole& enrole, Sender& sender )
    {
        boost::interprocess::basic_vectorbuf< mega::service::PacketBuffer > vectorBuffer;
        boost::archive::binary_oarchive oa(vectorBuffer, boostArchiveFlags);

        oa << mega::service::MessageType::eEnrole;
        oa << enrole;

        sender.send(vectorBuffer.vector());
    }

    inline void sendRegistration( const Registration& registration, Sender& sender )
    {
        boost::interprocess::basic_vectorbuf< mega::service::PacketBuffer > vectorBuffer;
        boost::archive::binary_oarchive oa(vectorBuffer, boostArchiveFlags);

        oa << mega::service::MessageType::eRegistry;
        oa << registration;

        sender.send(vectorBuffer.vector());
    }
}

