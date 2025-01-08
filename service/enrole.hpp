
#pragma once

#include "vocab/service/mp.hpp"

#include "common/serialisation.hpp"

#include <ostream>

namespace mega::service
{
class Enrole
{
public:
    Enrole() {}

    Enrole( MP daemon, MP mp )
        : m_daemon( daemon )
        , m_mp( mp )
    {
    }

    const MP getDaemon() const { return m_daemon; }
    const MP getMP() const { return m_mp; }

    template < class Archive >
    inline void serialize( Archive& archive, const unsigned int )
    {
        if constexpr( boost::serialization::IsXMLArchive<
                          Archive >::value )
        {
            archive& boost::serialization::make_nvp(
                "daemon", m_daemon );
            archive& boost::serialization::make_nvp( "mp", m_mp );
        }
        else
        {
            archive & m_daemon;
            archive & m_mp;
        }
    }

private:
    MP m_daemon{};
    MP m_mp{};
};

inline std::ostream& operator<<( std::ostream& os,
                                 const Enrole& enrole )
{
    return os << "daemon: " << enrole.getDaemon()
              << " mp: " << enrole.getMP();
}

} // namespace mega::service
