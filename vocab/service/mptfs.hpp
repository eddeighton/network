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

#ifndef GUARD_2025_January_07_mptfs
#define GUARD_2025_January_07_mptfs

#include "vocab/service/inline.h"

#include "vocab/native_types.hpp"

#include "vocab/service/mp.hpp"
#include "vocab/service/machine_id.hpp"
#include "vocab/service/process_id.hpp"
#include "vocab/service/thread_id.hpp"
#include "vocab/service/fiber_id.hpp"
#include "vocab/service/stack_id.hpp"
#include "vocab/service/mptf.hpp"

#include "common/serialisation.hpp"

#include <limits>
#include <ostream>
#include <iomanip>

namespace mega::service
{

class MPTFS : public c_machine_process_thread_fiber_stack_id
{
public:
    using ValueType = U64;

    struct Hash
    {
        inline U64 operator()( const MPTFS& ref ) const noexcept { return ref.getValue(); }
    };

    constexpr inline MPTFS()
        : c_machine_process_thread_fiber_stack_id{ 0, 0, 0, 0, 0 }
    {
    }

    constexpr inline explicit MPTFS( c_machine_id machineID, c_process_id processID, c_thread_id threadID, c_fiber_id fiberID, c_stack_id stackID )
        : c_machine_process_thread_fiber_stack_id(
            c_machine_process_thread_fiber_stack_id_make( machineID.value, processID.value, threadID.value, fiberID.value, stackID.value ) )
    {
    }

    constexpr inline explicit MPTFS( MachineID machineID, ProcessID processID, ThreadID threadID, FiberID fiberID, StackID stackID )
        : c_machine_process_thread_fiber_stack_id(
            c_machine_process_thread_fiber_stack_id_make( machineID.getValue(), processID.getValue(), threadID.getValue(), fiberID.getValue(), stackID.getValue() ) )
    {
    }

    constexpr inline explicit MPTFS( MPTF mptf, StackID stackID)
        : c_machine_process_thread_fiber_stack_id( c_machine_process_thread_fiber_stack_id_make(
            mptf.getMachineID().getValue(), mptf.getProcessID().getValue(), mptf.getThreadID().getValue(), mptf.getFiberID().getValue(), stackID.getValue() ) )
    {
    }

    constexpr inline explicit MPTFS( MP mp, ThreadID threadID, FiberID fiberID, StackID stackID)
        : c_machine_process_thread_fiber_stack_id( c_machine_process_thread_fiber_stack_id_make(
            mp.getMachineID().getValue(), mp.getProcessID().getValue(), threadID.getValue(), fiberID.getValue(), stackID.getValue() ) )
    {
    }

    constexpr inline explicit MPTFS( ValueType _value )
        : c_machine_process_thread_fiber_stack_id( c_machine_process_thread_fiber_stack_id_from_int( _value ) )
    {
    }

    constexpr inline MPTFS( const MPTFS& cpy ) = default;

    constexpr inline ValueType getValue() const { return c_machine_process_thread_fiber_stack_id_as_int( *this ); }
    constexpr inline MachineID getMachineID() const { return MachineID{ m_machine_id }; }
    constexpr inline ProcessID getProcessID() const { return ProcessID{ m_process_id }; }
    constexpr inline ThreadID  getThreadID() const { return ThreadID{ m_thread_id }; }
    constexpr inline FiberID   getFiberID() const { return FiberID{ m_fiber_id }; }
    constexpr inline StackID   getStackID() const { return StackID{ m_stack_id }; }

    constexpr inline MP   getMP()   const { return MP{ m_machine_id, m_process_id }; }
    constexpr inline MPTF getMPTF() const { return MPTF{ m_machine_id, m_process_id, m_thread_id, m_fiber_id }; }

    constexpr inline bool valid() const { return getValue() != 0; }

    constexpr inline bool operator==( const MPTFS& cmp ) const { return getValue() == cmp.getValue(); }
    constexpr inline bool operator!=( const MPTFS& cmp ) const { return !this->operator==( cmp ); }
    constexpr inline bool operator<( const MPTFS& cmp ) const { return getValue() < cmp.getValue(); }

    template < class Archive >
    inline void serialize( Archive& archive, const unsigned int )
    {
        if constexpr( boost::serialization::IsXMLArchive< Archive >::value )
        {
            archive& boost::serialization::make_nvp( "machine_id", m_machine_id.value );
            archive& boost::serialization::make_nvp( "process_id", m_process_id.value );
            archive& boost::serialization::make_nvp( "thread_id",  m_thread_id.value );
            archive& boost::serialization::make_nvp( "fiber_id",   m_fiber_id.value );
            archive& boost::serialization::make_nvp( "stack_id",   m_stack_id.value );
        }
        else
        {
            if constexpr( Archive::is_saving::value )
            {
                const ValueType v = getValue();
                archive << v;
            }
            else
            {
                ValueType v{};
                archive >> v;
                *this = MPTFS{v};
            }
        }
    }
};

static_assert( sizeof( MPTFS ) == sizeof( MPTFS::ValueType ), "Invalid MPTFS Size" );

inline std::ostream& operator<<( std::ostream& os, const MPTFS& value )
{
    return os << value.getMachineID() 
        << '.' << value.getProcessID() 
        << '.' << value.getThreadID() 
        << '.' << value.getFiberID() 
        << '.' << value.getStackID();
}

inline std::istream& operator>>( std::istream& is, MPTFS& mptfs )
{
    MachineID machineID;
    ProcessID processID;
    ThreadID  threadID;
    FiberID   fiberID;
    StackID   stackID;
    char      c;
    is >> machineID >> c >> processID >> c >> threadID >> c >> fiberID >> c >> stackID;
    mptfs = MPTFS{ machineID, processID, threadID, fiberID, stackID };
    return is;
}

} // namespace mega::service

#endif // GUARD_2025_January_07_mptfs

