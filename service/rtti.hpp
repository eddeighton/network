
#pragma once

#include "vocab/service/interface_type_name.hpp"

#include "common/serialisation.hpp"

#include <boost/type_index.hpp>

#include <string>
#include <vector>

namespace mega::service
{
class RTTI
{
public:
    using InterfaceTypeNameVector = std::vector< InterfaceTypeName >;

    InterfaceTypeNameVector m_interfaces;

    inline bool operator<( const RTTI& cmp ) const
    {
        return m_interfaces < cmp.m_interfaces;
    }

    inline bool contains( const InterfaceTypeName& name ) const
    {
        return std::find(
                   m_interfaces.begin(), m_interfaces.end(), name )
               != m_interfaces.end();
    }

    template < class Archive >
    inline void serialize( Archive& archive, const unsigned int )
    {
        if constexpr( boost::serialization::IsXMLArchive<
                          Archive >::value )
        {
            archive& boost::serialization::make_nvp(
                "interfaces", m_interfaces );
        }
        else
        {
            archive & m_interfaces;
        }
    }
};

template < typename T >
InterfaceTypeName getInterfaceTypeName()
{
    return boost::typeindex::type_id< T >().pretty_name();
}
} // namespace mega::service
