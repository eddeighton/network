
#pragma once

#include "service/enrole.hpp"
#include "service/registration.hpp"
#include "service/connection.hpp"

#include "service/protocol/message.hpp"
#include "service/protocol/serialization.hpp"

#include <set>

namespace mega::service
{
inline void sendEnrole( const Enrole&    enrole,
                        Connection::Ptr& pConnection )
{
    boost::interprocess::basic_vectorbuf<
        mega::service::PacketBuffer >
                                    vectorBuffer;
    boost::archive::binary_oarchive oa(
        vectorBuffer, boostArchiveFlags );

    oa << mega::service::MessageType::eEnrole;
    oa << enrole;

    pConnection->send( vectorBuffer.vector() );
}

inline void sendRegistration( const Registration& registration,
                              std::set< MP >      mps,
                              Connection::Ptr&    pConnection )
{
    boost::interprocess::basic_vectorbuf<
        mega::service::PacketBuffer >
                                    vectorBuffer;
    boost::archive::binary_oarchive oa(
        vectorBuffer, boostArchiveFlags );

    oa << mega::service::MessageType::eRegistry;
    oa << mps;
    oa << registration;

    pConnection->send( vectorBuffer.vector() );
}

inline void sendDisconnect( const MP& mp, std::set< MP > mps,
                            Connection::Ptr& pConnection )
{
    boost::interprocess::basic_vectorbuf<
        mega::service::PacketBuffer >
                                    vectorBuffer;
    boost::archive::binary_oarchive oa(
        vectorBuffer, boostArchiveFlags );

    oa << mega::service::MessageType::eDisconnect;
    oa << mps;
    oa << mp;

    pConnection->send( vectorBuffer.vector() );
}
} // namespace mega::service
