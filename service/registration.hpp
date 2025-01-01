
#pragma once

#include "service/rtti.hpp"

#include "vocab/service/mptfo.hpp"
#include "vocab/service/interface_type_name.hpp"

#include "common/serialisation.hpp"

#include <vector>
#include <tuple>

namespace mega::service
{
    class Registration
    {
    public:
        struct Registrant
        {
            MPTFO m_mptfo;
            RTTI m_rtti;

            using RegistrantVector = std::vector<Registrant>;

            inline bool operator<(const Registrant& cmp) const
            {
                return std::tie(m_mptfo, m_rtti) < std::tie(cmp.m_mptfo, cmp.m_rtti);
            }

            template < class Archive >
            inline void serialize( Archive& archive, const unsigned int )
            {
                if constexpr( boost::serialization::IsXMLArchive< Archive >::value )
                {
                    archive& boost::serialization::make_nvp( "mptfo", m_mptfo );
                    archive& boost::serialization::make_nvp( "rtti", m_rtti );
                }
                else
                {
                    archive& m_mptfo;
                    archive& m_rtti;
                }
            }
        };

        inline void filter(MP mp)
        {
            auto itEnd = std::remove_if(m_registrants.begin(), m_registrants.end(),
                [mp](const Registrant& registrant) -> bool
                {
                    return registrant.m_mptfo.getMP() == mp;
                });
            m_registrants.erase(itEnd, m_registrants.end());
        }

        template < class Archive >
        inline void serialize( Archive& archive, const unsigned int )
        {
            if constexpr( boost::serialization::IsXMLArchive< Archive >::value )
            {
                archive& boost::serialization::make_nvp( "registrants", m_registrants );
            }
            else
            {
                archive& m_registrants;
            }
        }

        Registrant::RegistrantVector m_registrants;
    };
}

