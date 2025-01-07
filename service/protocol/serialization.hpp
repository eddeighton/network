
#pragma once

#include "common/serialisation.hpp"

#include <boost/interprocess/interprocess_fwd.hpp>
#include <boost/interprocess/streams/vectorstream.hpp>

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>

namespace mega::service
{
    class Access;

    static constexpr auto boostArchiveFlags =
        boost::archive::no_header |
        boost::archive::no_codecvt |
        boost::archive::no_xml_tag_checking |
        boost::archive::no_tracking;

    using VectorBuffer = boost::interprocess::basic_vectorbuf< PacketBuffer >;

    class IArchive : public VectorBuffer, public boost::archive::binary_iarchive
    {
        Access& m_access;
    public:
        IArchive(Access& access, const PacketBuffer& buffer)
            :   VectorBuffer( buffer )
            ,   boost::archive::binary_iarchive( *static_cast<VectorBuffer*>(this), boostArchiveFlags )
            ,   m_access( access )
        {
        }

        Access& access() { return m_access; }
    };

    class OArchive : public VectorBuffer, public boost::archive::binary_oarchive
    {
        Access& m_access;
    public:
        OArchive(Access& access)
            :   VectorBuffer()
            ,   boost::archive::binary_oarchive( *static_cast<VectorBuffer*>(this), boostArchiveFlags )
            ,   m_access( access )
        {
        }

        Access& access() { return m_access; }
    };
}

