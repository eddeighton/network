

#pragma once

#include "round_robin.hpp"
#include "yield.hpp"

#include <boost/fiber/all.hpp>

namespace mega::service
{
    using IOContextPtr  = std::shared_ptr< boost::asio::io_context >;
    using SocketPtr     = std::shared_ptr< boost::asio::ip::tcp::socket >;

   
    inline void init_fiber_scheduler(IOContextPtr pIOContext)
    {
        boost::fibers::use_scheduling_algorithm< boost::fibers::asio::round_robin >(pIOContext);
    }


}

