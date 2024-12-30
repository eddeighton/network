
#pragma once

#include "vocab/service/mptfo.hpp"
#include "vocab/service/interface_type_name.hpp"

#include "common/serialisation.hpp"

namespace mega::service
{
    class Registration
    {
    public:
        MPTFO m_mptfo;
        std::vector< InterfaceTypeName > m_interfaces;

        template < class Archive >
        inline void serialize( Archive& archive, const unsigned int )
        {
            if constexpr( boost::serialization::IsXMLArchive< Archive >::value )
            {
                archive& boost::serialization::make_nvp( "mptfo", m_mptfo );
                archive& boost::serialization::make_nvp( "interfaces", m_interfaces );
            }
            else
            {
                archive& m_mptfo;
                archive& m_interfaces;
            }
        }
    };
}

