
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

#ifndef GUARD_2024_January_11_inline
#define GUARD_2024_January_11_inline

#include "vocab/runtime.h"
#include "vocab/native_types.h"
#include "vocab/compilation/concrete/inline.h"
#include "vocab/service/inline.h"

C_RUNTIME_START

/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////
// c_allocation_id
typedef struct c_allocation_id
{
    c_u16 value;
} c_allocation_id;

/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////
// c_allocation_id
typedef struct c_ref_count
{
    c_u16 value;
} c_ref_count;

/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////
// c_time_stamp
typedef struct _c_time_stamp
{
    c_u32 m_value;
} c_time_stamp;

inline c_time_stamp c_time_stamp_increment( c_time_stamp t )
{
    return c_time_stamp{ t.m_value + 1 };
}

inline c_bool c_time_stamp_less_than( c_time_stamp left, c_time_stamp right )
{
    return left.m_value < right.m_value;
}
inline c_bool c_time_stamp_equal_to( c_time_stamp left, c_time_stamp right )
{
    return left.m_value == right.m_value;
}

/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////
// c_pointer_net
#pragma pack( 1 )
typedef struct _c_pointer_net
{
    c_allocation_id             m_allocationID; // 2
    c_machine_id                m_machineID;    // 4
    c_process_id                m_processID;    // 1 
    c_thread_id                 m_threadID;     // 1 
    c_fiber_id                  m_fiberID;      // 1
    c_object_id                 m_objectID;     // 1
    c_concrete_type_id_instance m_type;         // 6
} c_pointer_net;
#pragma pack()
#ifndef MEGAJIT
static_assert( sizeof( c_pointer_net ) == 16U, "Invalid c_pointer_net Size" );
#endif

/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////
// c_object_header
#pragma pack( 1 )
typedef struct _c_object_header
{
    void*           m_buffer;
    void*           m_dallocator;
    c_pointer_net   m_pointer_net;
    c_time_stamp    m_lock_cycle;
    c_ref_count     m_ref_count;
} c_object_header;
#pragma pack()
#ifndef MEGAJIT
#endif

/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////
// c_pointer_heap
#pragma pack( 1 )
typedef struct _c_pointer_heap
{
    c_object_header*            m_header;   // 8
    c_u8                        _padding; // 1
    c_u8                        m_flags;  // 1
    c_concrete_type_id_instance m_type;   // 6
} c_pointer_heap;
#pragma pack()
#ifndef MEGAJIT
static_assert( sizeof( c_pointer_heap ) == 16U, "Invalid c_pointer_heap Size" );
#endif

/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////
// c_pointer_union
#pragma pack( 1 )
typedef union _pointer_union
{
    c_pointer_heap heap;
    c_pointer_net  net;
} c_pointer_union;
#pragma pack()

#ifndef MEGAJIT
static_assert( sizeof( c_pointer_union ) == 16U, "Invalid pointer_union Size" );
#endif

#pragma pack( 1 )
typedef struct _c_pointer
{
    c_pointer_union value;
} c_pointer;
#pragma pack()

#ifndef MEGAJIT
static_assert( sizeof( c_pointer ) == 16U, "Invalid c_pointer Size" );
#endif

C_RUNTIME_END
#endif // GUARD_2024_January_11_inline
