
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

#ifndef GUARD_2023_December_12_type_id
#define GUARD_2023_December_12_type_id

#include "vocab/compilation/interface/inline.h"

#include "vocab/compilation/interface/object_id.hpp"
#include "vocab/compilation/interface/sub_object_id.hpp"

#include "vocab/native_types.hpp"

#include "common/serialisation.hpp"

#include <limits>
#include <ostream>
#include <iomanip>

namespace mega::interface
{

class TypeID : public c_interface_type_id
{
public:
    using ValueType = U32;

    struct Hash
    {
        inline U64 operator()( const TypeID& ref ) const noexcept { return ref.getValue(); }
    };

    constexpr inline TypeID()
        : c_interface_type_id{ 0, 0 }
    {
    }
    constexpr inline explicit TypeID( c_interface_type_id _value )
        : c_interface_type_id( _value )
    {
    }
    constexpr inline explicit TypeID( ObjectID object, SubObjectID subObject )
        : c_interface_type_id( c_interface_type_id_make( object.getValue(), subObject.getValue() ) )
    {
    }
    constexpr inline explicit TypeID( ValueType _value )
        : c_interface_type_id( c_interface_type_id_from_int( _value ) )
    {
    }

    constexpr inline TypeID( const TypeID& cpy ) = default;

    constexpr inline bool isObject() const { return sub_object_id.value == 0; }

    constexpr inline ValueType   getValue() const { return c_interface_type_id_as_int( *this ); }
    constexpr inline ObjectID    getObjectID() const { return ObjectID{ object_id }; }
    constexpr inline SubObjectID getSubObjectID() const { return SubObjectID{ sub_object_id }; }

    constexpr inline bool valid() const { return getValue() != 0; }

    constexpr inline bool operator==( const TypeID& cmp ) const { return getValue() == cmp.getValue(); }
    constexpr inline bool operator!=( const TypeID& cmp ) const { return !this->operator==( cmp ); }
    constexpr inline bool operator<( const TypeID& cmp ) const { return getValue() < cmp.getValue(); }

    template < class Archive >
    inline void serialize( Archive& archive, const unsigned int )
    {
        if constexpr( boost::serialization::IsXMLArchive< Archive >::value )
        {
            archive& boost::serialization::make_nvp( "object_id", object_id.value );
            archive& boost::serialization::make_nvp( "sub_object_id", sub_object_id.value );
        }
        else
        {
            archive& object_id.value;
            archive& sub_object_id.value;
        }
    }
};

static_assert( sizeof( TypeID ) == sizeof( U32 ), "Invalid ObjectID Size" );

inline constexpr TypeID operator""_IT( unsigned long long int value )
{
    return TypeID( static_cast< TypeID::ValueType >( value ) );
}

static constexpr TypeID NULL_TYPE_ID  = 0x00000000_IT;
static constexpr TypeID OWNER_TYPE_ID = 0x00000001_IT;
static constexpr TypeID STATE_TYPE_ID = 0x00000002_IT;
static constexpr TypeID ROOT_TYPE_ID  = 0x00010000_IT;

static_assert( ROOT_TYPE_ID.getValue() == 0x00010000 );
static_assert( NULL_TYPE_ID.getValue() == 0x00000000 );

static_assert( ROOT_TYPE_ID.valid() );
static_assert( !NULL_TYPE_ID.valid() );

static_assert( NULL_TYPE_ID == TypeID( NULL_OBJECT_ID, NULL_SUB_OBJECT_ID ) );
static_assert( ROOT_TYPE_ID == TypeID( ROOT_OBJECT_ID, NULL_SUB_OBJECT_ID ) );

static_assert( ROOT_TYPE_ID != NULL_TYPE_ID );

static_assert( NULL_TYPE_ID.getObjectID() == NULL_OBJECT_ID );
static_assert( ROOT_TYPE_ID.getObjectID() == ROOT_OBJECT_ID );

static_assert( NULL_TYPE_ID.getSubObjectID() == NULL_SUB_OBJECT_ID );
static_assert( ROOT_TYPE_ID.getSubObjectID() == NULL_SUB_OBJECT_ID );

inline std::ostream& operator<<( std::ostream& os, const TypeID& value )
{
    return os << "0x" << std::hex << std::setw( 8 ) << std::setfill( '0' ) << value.getValue() << "_IT";
}

inline std::istream& operator>>( std::istream& is, TypeID& typeID )
{
    TypeID::ValueType value;
    is >> std::hex >> value;
    typeID = TypeID{ value };
    return is;
}

} // namespace mega::interface

#endif // GUARD_2023_December_12_type_id
