
#pragma once

#include "service/rtti.hpp"

#include "vocab/service/mptfo.hpp"
#include "vocab/service/interface_type_name.hpp"

#include "common/serialisation.hpp"

#include <vector>

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

