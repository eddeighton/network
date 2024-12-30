
#pragma once

#include "vocab/service/process_id.hpp"

#include "common/serialisation.hpp"

namespace mega::service
{
    class Enrole
    {
    public:
        Enrole( ProcessID processID )
            : m_processID( processID )
        {
        }

        const ProcessID getProcessID() const { return m_processID; }
   
        template < class Archive >
        inline void serialize( Archive& archive, const unsigned int )
        {
            if constexpr( boost::serialization::IsXMLArchive< Archive >::value )
            {
                archive& boost::serialization::make_nvp( "processID", m_processID );
            }
            else
            {
                archive& m_processID;
            }
        }

    private:
        ProcessID m_processID;
    };

}

