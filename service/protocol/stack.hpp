
//Sat Jan  4 13:04:33 UTC 2025
#pragma once

#include "vocab/service/mptfs.hpp"

#include "common/serialisation.hpp"

#include <vector>

namespace mega::service
{
    struct Stack
    {
        using MPTFSVector = std::vector< MPTFS >;
        MPTFSVector m_stack;

        inline bool operator<( const Stack& cmp ) const
        {
            return m_stack < cmp.m_stack;
        }

        inline bool operator==( const Stack& cmp ) const
        {
            return m_stack == cmp.m_stack;
        }

        inline void push_back( MPTFS mptfs )
        {
            m_stack.push_back( mptfs );
        }
        inline void push_back( MPTF mptf )
        {
            if( m_stack.empty() )
            {
                m_stack.push_back( MPTFS{ mptf, StackID{} } );
            }
            else
            {
                m_stack.push_back( MPTFS{ mptf, m_stack.back().getStackID() } );
            }
        }
        inline void pop_back()
        {
            m_stack.pop_back();
        }

        template < class Archive >
        void serialize( Archive& archive, const unsigned int )
        {
            if constexpr( boost::serialization::IsXMLArchive< Archive >::value )
            {
                archive& boost::serialization::make_nvp( "stack", m_stack );
            }
            else
            {
                archive& m_stack;
            }
        }
    };
}

