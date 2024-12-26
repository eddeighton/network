

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

#ifndef GUARD_2025_January_11_inline
#define GUARD_2025_January_11_inline

#include "vocab/runtime.h"
#include "vocab/native_types.h"
#include "vocab/compilation/concrete/inline.h"

C_RUNTIME_START

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
typedef struct c_machine_id
{
    c_u32 value;
} c_machine_id;

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
typedef struct _c_process_id
{
    c_u16 value;
} c_process_id;

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
typedef struct _c_owner_id
{
    c_u8 value;
} c_owner_id;

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
typedef struct _c_thread_id
{
    c_u8 value;
} c_thread_id;

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
typedef struct _c_fiber_id
{
    c_u8 value;
} c_fiber_id;

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
#pragma pack( 1 )
typedef struct _c_machine_process_id
{
    c_machine_id m_machine_id;
    c_process_id m_process_id;
    c_u16        m_padding;
} c_machine_process_id;
#pragma pack()

#ifndef MEGAJIT
static_assert( sizeof( c_machine_process_id ) == 8U, "Invalid c_machine_process_id Size" );
#endif

#ifdef __cplusplus
constexpr
#endif
    inline c_machine_process_id
    c_machine_process_id_make( c_u32 machine_id, c_u16 process_id )
{
    c_machine_process_id result;
    result.m_machine_id.value = machine_id;
    result.m_process_id.value = process_id;
    result.m_padding          = 0;
    return result;
}

#ifdef __cplusplus
constexpr
#endif
    inline c_u64
    c_machine_process_id_as_int( c_machine_process_id id )
{
    return ( ( c_u64 )id.m_machine_id.value ) + ( ( ( c_u64 )id.m_process_id.value ) << 32 );
}

#ifdef __cplusplus
constexpr
#endif
    inline c_machine_process_id
    c_machine_process_id_from_int( c_u64 i )
{
    return c_machine_process_id{ ( c_u32 )( i ), ( c_u16 )( i >> 32 ), 0 };
}

/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////
// c_machine_process_owner_id
#pragma pack( 1 )
typedef struct _c_machine_process_owner_id
{
    c_machine_id m_machine_id;
    c_process_id m_process_id;
    c_owner_id   m_owner_id;
    c_u8         m_padding;
} c_machine_process_owner_id;
#pragma pack()

#ifndef MEGAJIT
static_assert( sizeof( c_machine_process_owner_id ) == 8U, "Invalid c_machine_process_owner_id Size" );
#endif

#ifdef __cplusplus
constexpr
#endif
    inline c_machine_process_owner_id
    c_machine_process_owner_id_make( c_u32 machine_id, c_u16 process_id, c_u8 owner_id )
{
    c_machine_process_owner_id result;
    result.m_machine_id.value = machine_id;
    result.m_process_id.value = process_id;
    result.m_owner_id.value   = owner_id;
    result.m_padding          = 0;
    return result;
}

#ifdef __cplusplus
constexpr
#endif
    inline c_u64
    c_machine_process_owner_id_as_int( c_machine_process_owner_id id )
{
    return ( ( c_u64 )id.m_machine_id.value ) + ( ( ( c_u64 )id.m_process_id.value ) << 32 )
           + ( ( ( c_u64 )id.m_owner_id.value ) << 48 );
}

#ifdef __cplusplus
constexpr
#endif
    inline c_machine_process_owner_id
    c_machine_process_owner_id_from_int( c_u64 i )
{
    return c_machine_process_owner_id{ ( c_u32 )( i ), ( c_u16 )( i >> 32 ), ( c_u8 )( i >> 48 ), 0 };
}

/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////
// c_machine_process_thread_fiber_id
#pragma pack( 1 )
typedef struct _c_machine_process_thread_fiber_id
{
    c_machine_id m_machine_id;
    c_process_id m_process_id;
    c_thread_id  m_thread_id;
    c_fiber_id   m_fiber_id;
} c_machine_process_thread_fiber_id;
#pragma pack()

#ifndef MEGAJIT
static_assert( sizeof( c_machine_process_thread_fiber_id ) == 8U, "Invalid c_machine_process_thread_fiber_id Size" );
#endif

#ifdef __cplusplus
constexpr
#endif
    inline c_machine_process_thread_fiber_id
    c_machine_process_thread_fiber_id_make( c_u32 machine_id, c_u16 process_id, c_u8 thread_id, c_u8 fiber_id )
{
    c_machine_process_thread_fiber_id result;
    result.m_machine_id.value = machine_id;
    result.m_process_id.value = process_id;
    result.m_thread_id.value  = thread_id;
    result.m_fiber_id.value   = fiber_id;
    return result;
}

#ifdef __cplusplus
constexpr
#endif
    inline c_u64
    c_machine_process_thread_fiber_id_as_int( c_machine_process_thread_fiber_id id )
{
    return     ( ( c_u64 )id.m_machine_id.value )
           + ( ( ( c_u64 )id.m_process_id.value ) << 32 )
           + ( ( ( c_u64 )id.m_thread_id.value  ) << 48 )
           + ( ( ( c_u64 )id.m_fiber_id.value   ) << 56 );
}

#ifdef __cplusplus
constexpr
#endif
    inline c_machine_process_thread_fiber_id
    c_machine_process_thread_fiber_id_from_int( c_u64 i )
{
    return c_machine_process_thread_fiber_id{ ( c_u32 )( i ), ( c_u16 )( i >> 32 ), ( c_u8 )( i >> 48 ), ( c_u8 )( i >> 56 ) };
}

C_RUNTIME_END
#endif // GUARD_2025_January_11_inline
