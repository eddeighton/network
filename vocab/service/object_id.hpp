
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

#ifndef GUARD_2024_January_11_object_id
#define GUARD_2024_January_11_object_id

#include "vocab/runtime/inline.h"

#include "vocab/native_types.hpp"

#include "common/serialisation.hpp"

#include <ostream>
#include <istream>

namespace mega::service
{

class ObjectID : public c_object_id
{
public:
    using ValueType = U8;

    struct Hash
    {
        inline U64 operator()( const c_object_id& value ) const noexcept
        {
            return value.value;
        }
    };

    constexpr inline ObjectID()
        : c_object_id{ 0U }
    {
    }

    constexpr inline explicit ObjectID( c_object_id _value )
        : c_object_id( _value )
    {
    }

    constexpr inline explicit ObjectID( ValueType _value )
        : c_object_id{ _value }
    {
    }

    constexpr inline ObjectID( const ObjectID& cpy )            = default;
    constexpr inline ObjectID( ObjectID&& cpy )                 = default;
    constexpr inline ObjectID& operator=( const ObjectID& cpy ) = default;

    constexpr inline ValueType getValue() const { return value; }

    constexpr inline bool operator<( const ObjectID& cpy ) const { return value < cpy.value; }
    constexpr inline bool operator==( const ObjectID& cpy ) const { return value == cpy.value; }
    constexpr inline bool operator!=( const ObjectID& cpy ) const { return !this->operator==( cpy ); }

    // post increment
    constexpr inline ObjectID operator++(int)
    {
        ObjectID temp = *this;
        value = static_cast< ValueType >( static_cast<int>(value) + 1 );
        return temp;
    }
    // pre increment
    constexpr inline ObjectID operator++()
    {
        value = static_cast< ValueType >( static_cast<int>(value) + 1 );
        return *this;
    }

    template < class Archive >
    inline void serialize( Archive& archive, const unsigned int )
    {
        if constexpr( boost::serialization::IsXMLArchive< Archive >::value )
        {
            archive& boost::serialization::make_nvp( "object_id", value );
        }
        else
        {
            archive& value;
        }
    }
};

static_assert( sizeof( ObjectID ) == sizeof( ObjectID::ValueType ), "Invalid ObjectID Size" );

inline constexpr ObjectID operator""_O( unsigned long long int value )
{
    return ObjectID{ static_cast< ObjectID::ValueType >( value ) };
}

static constexpr ObjectID OBJECT_ZERO = 0x00_O;

inline std::ostream& operator<<( std::ostream& os, const ObjectID& instance )
{
    return os << "0x" << std::hex << std::right << std::setw( 2 ) << std::setfill( '0' ) << 
        static_cast< int >( instance.getValue() ) << "_O";
}

inline std::istream& operator>>( std::istream& is, ObjectID& instance )
{
    int value;
    is >> value;
    instance = ObjectID{ static_cast< ObjectID::ValueType >( value ) };
    return is;
}

} // namespace mega::service

#endif // GUARD_2024_January_11_object_id
