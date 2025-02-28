
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

#include "vocab/compilation/hyper_graph.hpp"

#include "common/assert_verify.hpp"

#include <algorithm>
#include <array>
#include <string>

namespace
{
using namespace std::string_literals;
static const std::array< std::string, mega::EdgeType::TOTAL_EDGE_TYPES > g_edgeTypes = {

    "Parent"s,
    "ChildSingular"s,
    "ChildNonSingular"s,
    "Dim"s,
    "Link"s,
    "InterObjectNonOwner"s,
    "InterObjectOwner"s,
    "InterObjectParent"s
};
} // namespace

namespace mega
{

const char* EdgeType::str() const
{
    switch( m_value )
    {
        case eParent:
        case eChildSingular:
        case eChildNonSingular:
        case eDim:
        case eLink:
        case eInterObjectNonOwner:
        case eInterObjectOwner:
        case eInterObjectParent:
            return g_edgeTypes[ m_value ].c_str();
        case TOTAL_EDGE_TYPES:
        default:
            THROW_RTE( "Invalid EdgeType type" );
    }
}

EdgeType EdgeType::fromStr( const char* psz )
{
    auto iFind = std::find( g_edgeTypes.begin(), g_edgeTypes.end(), psz );
    VERIFY_RTE_MSG( iFind != g_edgeTypes.end(), "Unknown ownership mode: " << psz );
    return { static_cast< EdgeType::Value >( std::distance( g_edgeTypes.cbegin(), iFind ) ) };
}
} // namespace mega

std::ostream& operator<<( std::ostream& os, mega::EdgeType edgeType )
{
    return os << edgeType.str();
}
