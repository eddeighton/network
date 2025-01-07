

#pragma once

#include <boost/asio.hpp>
#include <boost/asio/use_future.hpp>

namespace mega::service
{
    using IOContextPtr = std::shared_ptr< boost::asio::io_context >;

    void init_fiber_scheduler(IOContextPtr pIOContext);
}

