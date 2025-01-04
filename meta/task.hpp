//  Copyright (c) Deighton Systems Limited. 2025. All Rights Reserved.
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

#pragma once

#include "meta/configuration.hpp"
#include "meta/environment.hpp"
#include "meta/pipeline/stash_environment.hpp"

#include "pipeline/stash.hpp"
#include "pipeline/pipeline.hpp"

#include "common/terminal.hpp"
#include "common/requireSemicolon.hpp"

#include <string>
#include <sstream>

namespace mega::meta
{

struct TaskDependencies
{
    meta::Configuration&            m_configuration;
    mega::io::Manifest&             m_manifest;
    mega::io::MetaStashEnvironment& m_environment;
    pipeline::Progress&             m_progress;
    pipeline::Stash&                m_stash;
};

inline std::string StartMsg(const char* pszTaskName)
{
    std::ostringstream os;
    os << common::COLOUR_YELLOW_BEGIN << 
        "START:   " << pszTaskName << common::COLOUR_END;
    return os.str();
}
inline std::string ProgressMsg(const char* pszTaskName)
{
    std::ostringstream os;
    os << common::COLOUR_BLUE_BEGIN << 
        "MSG:     " << pszTaskName << common::COLOUR_END;
    return os.str();
}
inline std::string CachedMsg(const char* pszTaskName)
{
    std::ostringstream os;
    os << common::COLOUR_MAGENTA_BEGIN << 
        "CACHED:  " << pszTaskName << common::COLOUR_END;
    return os.str();
}
inline std::string FailMsg(const char* pszTaskName)
{
    std::ostringstream os;
    os << common::COLOUR_RED_BEGIN << 
        "FAIL:    " << pszTaskName << common::COLOUR_END;
    return os.str();
}
inline std::string CompleteMsg(const char* pszTaskName)
{
    std::ostringstream os;
    os << common::COLOUR_GREEN_BEGIN << 
        "SUCCESS: " << pszTaskName << common::COLOUR_END;
    return os.str();
}

#define TASK_START(taskname) dependencies.m_progress.onStarted( meta::StartMsg( taskname ) )
#define TASK_CACHED(taskname) dependencies.m_progress.onCompleted( meta::CachedMsg( taskname ) )
#define TASK_FAIL(taskname) dependencies.m_progress.onFailed( meta::FailMsg( taskname ) )
#define TASK_COMPLETE(taskname) dependencies.m_progress.onCompleted( meta::CompleteMsg( taskname ) )

#define TASK_PROGRESS(taskname, msg)                                    \
    DO_STUFF_AND_REQUIRE_SEMI_COLON( std::ostringstream _os2;           \
                                     _os2 << common::COLOUR_BLUE_BEGIN  \
                                          << "MSG:     " << taskname    \
                                          << " " << msg                 \
                                          << common::COLOUR_END;        \
                       dependencies.m_progress.onProgress( _os2.str() );)

}

