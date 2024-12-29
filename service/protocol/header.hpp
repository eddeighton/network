
#pragma once

#include "vocab/service/object_id.hpp"
#include "vocab/service/mptf.hpp"
#include "vocab/service/interface_type_name.hpp"
#include "vocab/service/function_type_name.hpp"

#include "common/serialisation.hpp"

#include <cstdint>
#include <iostream>

namespace mega::service
{
    using MessageID = std::uint64_t;

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

    inline std::ostream& operator<<( std::ostream& os, const MPTFO& mptf )
    {
        return os 
        << "MPTF : " << mptf.m_mptf
        << "ObjectID : " << mptf.m_objectID  
        ;
    }

    struct Header
    {
        MessageID m_messageID;
        MPTF m_requester;
        MPTFO m_responder;
        InterfaceTypeName m_interfaceName;
        FunctionTypeName m_functionName;

        template < class Archive >
        inline void serialize( Archive& archive, const unsigned int )
        {
            if constexpr( boost::serialization::IsXMLArchive< Archive >::value )
            {
                archive& boost::serialization::make_nvp( "messageID", m_messageID );
                archive& boost::serialization::make_nvp( "requester", m_requester );
                archive& boost::serialization::make_nvp( "responder", m_responder );
                archive& boost::serialization::make_nvp( "interfaceName", m_interfaceName );
                archive& boost::serialization::make_nvp( "functionName", m_functionName );
            }
            else
            {
                archive& m_messageID;
                archive& m_requester;
                archive& m_responder;
                archive& m_interfaceName;
                archive& m_functionName;
            }
        }
    };

    inline std::ostream& operator<<( std::ostream& os, const Header& header )
    {
        return os 
        << "Message ID : " << header.m_messageID 
        << "Requester MPTF : " << header.m_requester  
        << "Responder MPTF : " << header.m_responder  
        << "Interface Name : " << header.m_interfaceName  
        << "Function Name : " << header.m_functionName
        ;
    }
}

