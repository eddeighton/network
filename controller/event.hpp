

#pragma once

#include "libevdev/libevdev.h"

namespace mega::controller
{

struct EventGroup
{
    input_event scan, key, syn;
};

} // namespace mega::controller
