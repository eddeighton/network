
#pragma once

#include "vocab/service/object_id.hpp"
#include "vocab/service/mptf.hpp"
#include "vocab/service/interface_type_name.hpp"
#include "vocab/service/function_type_name.hpp"

#include "common/serialisation.hpp"

namespace mega::service
{
    struct MPTFO
    {
        MPTF     m_mptf;
        ObjectID m_objectID;

        template < class Archive >
        inline void serialize( Archive& archive, const unsigned int )
        {
            if constexpr( boost::serialization::IsXMLArchive< Archive >::value )
            {
                archive& boost::serialization::make_nvp( "mptf", m_mptf );
                archive& boost::serialization::make_nvp( "objectID", m_objectID );
            }
            else
            {
                archive& m_mptf;
                archive& m_objectID;
            }
        }
    };

    struct Header
    {
        MPTF m_requester;
        MPTFO m_responder;
        InterfaceTypeName m_interfaceName;
        FunctionTypeName m_functionName;

        template < class Archive >
        inline void serialize( Archive& archive, const unsigned int )
        {
            if constexpr( boost::serialization::IsXMLArchive< Archive >::value )
            {
                archive& boost::serialization::make_nvp( "requester", m_requester );
                archive& boost::serialization::make_nvp( "responder", m_responder );
                archive& boost::serialization::make_nvp( "interfaceName", m_interfaceName );
                archive& boost::serialization::make_nvp( "functionName", m_functionName );
            }
            else
            {
                archive& m_requester;
                archive& m_responder;
                archive& m_interfaceName;
                archive& m_functionName;
            }
        }
    };
}

