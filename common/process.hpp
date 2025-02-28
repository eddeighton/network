//  Copyright (c) Deighton Systems Limited. 2023. All Rights Reserved.
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

#ifndef GUARD_2023_November_13_process
#define GUARD_2023_November_13_process

#include <string>
#include <map>

namespace common
{

inline int runProcess( const std::string& strCmd, std::string& strOutput, std::string& strError );

struct Command
{
    using EnvironmentMap = std::map< std::string, std::string >;

    std::string    m_strCmd;
    EnvironmentMap m_environmentVars;

    std::string str() const;
};

inline int runCmd( const Command& cmd, std::string& strOutput, std::string& strError );

} // namespace common

#include "common/process.ipp"

#endif //GUARD_2023_November_13_process


