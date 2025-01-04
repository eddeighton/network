
//  Copyright (c) Deighton Systems Limited. 2022. All Rights Reserved.
//  Author: Edward Deighton
//  License: Please see license.txt in the project root folder.

//  Use and copying of this software and preparation of derivative works
//  based upon this software are permitted. Any copy of this software or
//  of any derivative work must include the above copyright notice, this
//  paragraph and the one after it.  Any distribution of this software or
//  derivative works must comply with all applicable laws.

//  This software is made available AS IS, and COPYRIGHT OWNERS DISCLAIMS
//  ALL WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE
//  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
//  PURPOSE, AND NOTWITHSTANDING ANY OTHER PROVISION CONTAINED HEREIN, ANY
//  LIABILITY FOR DAMAGES RESULTING FROM THE SOFTWARE OR ITS USE IS
//  EXPRESSLY DISCLAIMED, WHETHER ARISING IN CONTRACT, TORT (INCLUDING
//  NEGLIGENCE) OR STRICT LIABILITY, EVEN IF COPYRIGHT OWNERS ARE ADVISED
//  OF THE POSSIBILITY OF SUCH DAMAGES.

#ifndef GUARD_2025_January_11_stack_id
#define GUARD_2025_January_11_stack_id

#include "vocab/runtime/inline.h"

#include "vocab/native_types.hpp"

#include "common/serialisation.hpp"

#include <ostream>
#include <istream>

namespace mega::service
{

class StackID : public c_stack_id
{
public:
    using ValueType = U8;

    struct Hash
    {
        inline U64 operator()( const c_stack_id& value ) const noexcept
        {
            return value.value;
        }
    };

    constexpr inline StackID()
        : c_stack_id{ 0U }
    {
    }

    constexpr inline explicit StackID( c_stack_id _value )
        : c_stack_id( _value )
    {
    }

    constexpr inline explicit StackID( ValueType _value )
        : c_stack_id{ _value }
    {
    }

    constexpr inline StackID( const StackID& cpy )            = default;
    constexpr inline StackID( StackID&& cpy )                 = default;
    constexpr inline StackID& operator=( const StackID& cpy ) = default;

    constexpr inline ValueType getValue() const { return value; }

    constexpr inline bool operator<( const StackID& cpy ) const { return value < cpy.value; }
    constexpr inline bool operator==( const StackID& cpy ) const { return value == cpy.value; }
    constexpr inline bool operator!=( const StackID& cpy ) const { return !this->operator==( cpy ); }

    template < class Archive >
    inline void serialize( Archive& archive, const unsigned int )
    {
        if constexpr( boost::serialization::IsXMLArchive< Archive >::value )
        {
            archive& boost::serialization::make_nvp( "stack_id", value );
        }
        else
        {
            archive& value;
        }
    }
};

static_assert( sizeof( StackID ) == sizeof( StackID::ValueType ), "Invalid StackID Size" );

inline constexpr StackID operator""_S( unsigned long long int value )
{
    return StackID{ static_cast< StackID::ValueType >( value ) };
}

static constexpr StackID STACK_ZERO = 0x00_S;

inline std::ostream& operator<<( std::ostream& os, const StackID& instance )
{
    return os << "0x" << std::hex << std::setw( 2 ) << std::setfill( '0' ) << 
        static_cast< U32 >( instance.getValue() ) << "_S";
}

inline std::istream& operator>>( std::istream& is, StackID& instance )
{
    U32 value;
    is >> value;
    instance = StackID{ static_cast< StackID::ValueType >( value ) };
    return is;
}

} // namespace mega::service

#endif // GUARD_2025_January_11_stack_id
