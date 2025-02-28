
#pragma once

namespace mega::service
{
struct Interface
{
    virtual ~Interface() = 0;
};
inline Interface::~Interface() = default;

struct Factory : public virtual Interface
{
};
} // namespace mega::service
