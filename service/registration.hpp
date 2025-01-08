
#pragma once

#include "service/rtti.hpp"

#include "vocab/service/mptfo.hpp"
#include "vocab/service/interface_type_name.hpp"

#include "common/serialisation.hpp"

#include <set>
#include <tuple>
#include <ostream>

namespace mega::service
{
class Registration
{
public:
    struct Registrant
    {
        MPTFO m_mptfo;
        RTTI  m_rtti;

        using RegistrantVector = std::set< Registrant >;

        inline bool operator<( const Registrant& cmp ) const
        {
            return std::tie( m_mptfo, m_rtti )
                   < std::tie( cmp.m_mptfo, cmp.m_rtti );
        }

        template < class Archive >
        inline void serialize( Archive& archive, const unsigned int )
        {
            if constexpr( boost::serialization::IsXMLArchive<
                              Archive >::value )
            {
                archive& boost::serialization::make_nvp(
                    "mptfo", m_mptfo );
                archive& boost::serialization::make_nvp(
                    "rtti", m_rtti );
            }
            else
            {
                archive & m_mptfo;
                archive & m_rtti;
            }
        }
    };

    inline void remove( MP mp )
    {
        std::erase_if( m_registrants,
                       [ mp ]( const Registrant& registrant ) -> bool
                       { return registrant.m_mptfo.getMP() == mp; } );
    }

    inline void add( const Registration& addition )
    {
        m_registrants.insert( addition.m_registrants.begin(),
                              addition.m_registrants.end() );
    }

    template < class Archive >
    inline void serialize( Archive& archive, const unsigned int )
    {
        if constexpr( boost::serialization::IsXMLArchive<
                          Archive >::value )
        {
            archive& boost::serialization::make_nvp(
                "registrants", m_registrants );
        }
        else
        {
            archive & m_registrants;
        }
    }

    Registrant::RegistrantVector m_registrants;
};

inline std::ostream& operator<<( std::ostream&       os,
                                 const Registration& reg )
{
    for( const auto& registrant : reg.m_registrants )
    {
        os << "MPTFO: " << registrant.m_mptfo << "\n";
        for( const auto& interface : registrant.m_rtti.m_interfaces )
        {
            os << "  Interface: " << interface << "\n";
        }
    }
    return os;
}
} // namespace mega::service
