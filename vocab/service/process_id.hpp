
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

#ifndef GUARD_2024_January_11_process_id
#define GUARD_2024_January_11_process_id

#include "vocab/runtime/inline.h"

#include "vocab/native_types.hpp"

#include "common/serialisation.hpp"

#include "common/assert_verify.hpp"

#include <limits>
#include <ostream>
#include <istream>

namespace mega::service
{

class ProcessID : public c_process_id
{
public:
    using ValueType = U8;

    constexpr inline ProcessID()
        : c_process_id{ 0U }
    {
    }

    constexpr inline explicit ProcessID( c_process_id _value )
        : c_process_id( _value )
    {
    }

    constexpr inline explicit ProcessID( ValueType _value )
        : c_process_id{ _value }
    {
    }

    constexpr inline ProcessID( const ProcessID& cpy )            = default;
    constexpr inline ProcessID( ProcessID&& cpy )                 = default;
    constexpr inline ProcessID& operator=( const ProcessID& cpy ) = default;

    constexpr inline ValueType getValue() const { return value; }

    constexpr inline bool operator<( const ProcessID& cpy ) const { return value < cpy.value; }
    constexpr inline bool operator==( const ProcessID& cpy ) const { return value == cpy.value; }
    constexpr inline bool operator!=( const ProcessID& cpy ) const { return !this->operator==( cpy ); }

    // post increment
    constexpr inline ProcessID operator++(int)
    {
        ProcessID temp = *this;
        value = static_cast< ValueType >( static_cast<int>(value) + 1 );
        return temp;
    }
    // pre increment
    constexpr inline ProcessID operator++()
    {
        value = static_cast< ValueType >( static_cast<int>(value) + 1 );
        return *this;
    }

    template < class Archive >
    inline void serialize( Archive& archive, const unsigned int )
    {
        if constexpr( boost::serialization::IsXMLArchive< Archive >::value )
        {
            archive& boost::serialization::make_nvp( "process_id", value );
        }
        else
        {
            archive& value;
        }
    }
};

static_assert( sizeof( ProcessID ) == sizeof( ProcessID::ValueType ), "Invalid ProcessID Size" );

inline constexpr ProcessID operator""_P( unsigned long long int value )
{
    return ProcessID{ static_cast< ProcessID::ValueType >( value ) };
}

static constexpr ProcessID PROCESS_ZERO = 0x0000_P;
static constexpr ProcessID PROCESS_ONE  = 0x0001_P;

inline std::ostream& operator<<( std::ostream& os, const ProcessID& instance)
{
    return os << "0x" << std::hex << std::right << std::setw( 2 ) << std::setfill( '0' ) << 
        static_cast< int >(instance.getValue()) << std::dec << "_P";
}

inline std::istream& operator>>( std::istream& is, ProcessID& instance)
{
    int value;
    is >> value;
    instance = ProcessID{ static_cast< ProcessID::ValueType >( value ) };
    return is;
}

} // namespace mega::service

#endif // GUARD_2024_January_11_process_id
