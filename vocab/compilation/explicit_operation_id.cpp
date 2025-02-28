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

#include "vocab/compilation/explicit_operation_id.hpp"

#include "common/assert_verify.hpp"

#include <algorithm>
#include <limits>

namespace mega
{

// clang-format off
static const ExplicitOperationIDStringArray g_pszExplicitOperationStrings =
{
    std::string( "Read" ),
    std::string( "Write" ),

    std::string( "LinkRead" ),
    std::string( "LinkAdd" ),
    std::string( "LinkRemove" ),
    std::string( "LinkClear" ),

    std::string( "Call" ),
    std::string( "Signal" ),
    std::string( "Start" ),
    std::string( "Move" ),
    std::string( "GetContext" ),
    std::string( "Range" )
};
// clang-format on
static_assert( HIGHEST_EXPLICIT_OPERATION_TYPE == g_pszExplicitOperationStrings.size(),
               "Incorrect explicit operation strings" );

const std::string& getExplicitOperationString( ExplicitOperationID op )
{
    ASSERT( op < HIGHEST_EXPLICIT_OPERATION_TYPE );
    return g_pszExplicitOperationStrings[ op ];
}

ExplicitOperationID getExplicitOperationName( const std::string& strName )
{
    auto iFind = std::find( g_pszExplicitOperationStrings.begin(), g_pszExplicitOperationStrings.end(), strName );
    if( iFind == g_pszExplicitOperationStrings.end() )
        return HIGHEST_EXPLICIT_OPERATION_TYPE;
    else
        return static_cast< ExplicitOperationID >( std::distance( g_pszExplicitOperationStrings.begin(), iFind ) );
}

const ExplicitOperationIDStringArray& getExplicitOperationStrings()
{
    return g_pszExplicitOperationStrings;
}

} // namespace mega
