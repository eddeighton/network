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

#include "database/environment.hxx"
#include "database/directories.hpp"
#include "database/component_info.hpp"
#include "database/sources.hpp"
#include "database/serialisation.hpp"

#include "database/FirstStage.hxx"
#include "database/SecondStage.hxx"
#include "database/ThirdStage.hxx"
#include "database/FourthStage.hxx"
#include "database/manifest.hxx"
#include "database/environment.hxx"

#include "../environment_test.hpp"

#include "common/file.hpp"
#include "common/string.hpp"

#include <gtest/gtest.h>

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>

#include <fstream>
#include <memory>
#include <optional>

class FirstDBTest : public ::testing::Test
{
public:
    using Env = mega::io::TestEnvironment< mega::io::Environment >;
    boost::filesystem::path                  m_tempDir;
    std::unique_ptr< mega::io::Directories > m_pDirectories;
    std::unique_ptr< Env >                   m_pEnvironment;

    virtual void SetUp() override
    {
        m_tempDir = boost::filesystem::temp_directory_path() / "FirstDBTest" / common::uuid();
        boost::filesystem::create_directories( m_tempDir );

        m_pDirectories
            = std::make_unique< mega::io::Directories >( mega::io::Directories{ m_tempDir, m_tempDir, "", "" } );
        m_pEnvironment = std::make_unique< Env >( *m_pDirectories );

        std::vector< mega::io::ComponentInfo > componentInfos;
        const mega::io::Manifest               manifest( *m_pEnvironment, m_tempDir, componentInfos );
        const mega::io::manifestFilePath       projectManifestPath = m_pEnvironment->project_manifest();
        manifest.save_temp( *m_pEnvironment, projectManifestPath );
        m_pEnvironment->temp_to_real( projectManifestPath );
    }
    virtual void TearDown() override
    {
        m_pEnvironment.reset();
        namespace bfs = boost::filesystem;
        bfs::remove_all( m_tempDir );
    }
};

TEST_F( FirstDBTest, BasicTypes )
{
    using namespace FirstStage;
    using namespace FirstStage::TestNamespace;
}
