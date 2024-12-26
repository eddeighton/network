

#pragma once

#include <boost/asio.hpp>

namespace mega::service
{
    using IOContextPtr = std::shared_ptr< boost::asio::io_context >;

    void init_fiber_scheduler(IOContextPtr pIOContext);
}

