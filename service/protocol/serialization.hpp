
#pragma once

#include "common/serialisation.hpp"

#include <boost/interprocess/interprocess_fwd.hpp>
#include <boost/interprocess/streams/vectorstream.hpp>

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>

namespace mega::service
{
    static constexpr auto boostArchiveFlags =
        boost::archive::no_header |
        boost::archive::no_codecvt |
        boost::archive::no_xml_tag_checking |
        boost::archive::no_tracking;

}

