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

#include "vocab/service/node.hpp"

#include "common/processID.hpp"
#include "common/assert_verify.hpp"

#include <string>
#include <array>
#include <algorithm>
#include <sstream>

namespace mega::network
{
const char* ENV_PROCESS_UUID = "MEGA_PROCESS_UUID";

namespace
{
using NodeStringArray = std::array< std::string, Node::TOTAL_NODE_TYPES >;
// clang-format off
static const NodeStringArray g_nodeTypeStrings = 
{ 
    std::string{ "Leaf" }, 
    std::string{ "Terminal" }, 
    std::string{ "Tool" }, 
    std::string{ "Python" }, 
    std::string{ "Report" }, 
    std::string{ "Executor" }, 
    std::string{ "Plugin" }, 
    std::string{ "Daemon" },
    std::string{ "Root" } 
};
// clang-format on
} // namespace

const std::string& Node::str() const
{
    return g_nodeTypeStrings[ m_type ];
}

bool Node::canAllocateObjects() const
{
    switch( m_type )
    {
        case Tool:
        case Python:
        case Report:
        case Executor:
        case Plugin:
            return true;
        case Leaf:
        case Terminal:
        case Daemon:
        case Root:
            return false;
        case TOTAL_NODE_TYPES:
        default:
            THROW_RTE( "Unknown node type" );
            return false;
    }
}

Node Node::fromStr( const char* pszStr )
{
    auto iFind = std::find( g_nodeTypeStrings.begin(), g_nodeTypeStrings.end(), pszStr );
    if( iFind == g_nodeTypeStrings.end() )
        return Node{ TOTAL_NODE_TYPES };
    else
        return Node{ static_cast< Node::Type >( std::distance( g_nodeTypeStrings.begin(), iFind ) ) };
}

std::string Node::makeProcessName( Node node )
{
    std::ostringstream os;
    os << node.str() << "_" << common::ProcessID::get().getPID();
    return os.str();
}

} // namespace mega::network
