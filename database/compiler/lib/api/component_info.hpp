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

#ifndef IO_SOURCETREE_25_MARCH_2022
#define IO_SOURCETREE_25_MARCH_2022

#include "database/component_type.hpp"
#include "database/serialisation.hpp"

#include "common/assert_verify.hpp"
#include "common/file.hpp"

#include <boost/filesystem.hpp>

#include <vector>

namespace mega::io
{
class ComponentInfo
{
public:
    using PathArray = std::vector< boost::filesystem::path >;

    inline ComponentInfo() = default;

    inline ComponentInfo( ComponentType componentType, const std::string& strName,
                                  const boost::filesystem::path& filePath, const std::vector< std::string >& cppFlags,
                                  const std::vector< std::string >& cppDefines, const boost::filesystem::path& srcDir,
                                  const boost::filesystem::path& buildDir, const ComponentInfo::PathArray& sourceFiles,
                                  const ComponentInfo::PathArray& dependencyFiles, const PathArray& includeDirectories )
        : m_componentType( componentType )
        , m_strName( strName )
        , m_filePath( filePath )
        , m_cppFlags( cppFlags )
        , m_cppDefines( cppDefines )
        , m_srcDir( srcDir )
        , m_buildDir( buildDir )
        , m_sourceFiles( sourceFiles )
        , m_dependencyFiles( dependencyFiles )
        , m_includeDirectories( includeDirectories )
    {
    }

    static inline ComponentInfo load( const boost::filesystem::path& componentInfoPath )
    {
        io::ComponentInfo componentInfo;
        {
            VERIFY_RTE_MSG( boost::filesystem::exists( componentInfoPath ),
                            "Failed to locate file: " << componentInfoPath.string() );
            auto pInFileStream = boost::filesystem::loadFileStream( componentInfoPath );
            boost::archive::xml_iarchive ia( *pInFileStream );
            ia >> boost::serialization::make_nvp( "componentInfo", componentInfo );
        }
        return componentInfo;
    }

    inline ComponentType                     getComponentType() const { return m_componentType; }
    inline const std::string&                getName() const { return m_strName; }
    inline const boost::filesystem::path&    getFilePath() const { return m_filePath; }
    inline const std::vector< std::string >& getCPPFlags() const { return m_cppFlags; }
    inline const std::vector< std::string >& getCPPDefines() const { return m_cppDefines; }
    inline const boost::filesystem::path&    getSrcDir() const { return m_srcDir; }
    inline const boost::filesystem::path&    getBuildDir() const { return m_buildDir; }
    inline const PathArray&                  getSourceFiles() const { return m_sourceFiles; }
    inline const PathArray&                  getDependencyFiles() const { return m_dependencyFiles; }
    inline const PathArray&                  getIncludeDirectories() const { return m_includeDirectories; }

    template < class Archive >
    inline void serialize( Archive& archive, const unsigned int )
    {
        archive& boost::serialization::make_nvp( "componentType", m_componentType );
        archive& boost::serialization::make_nvp( "name", m_strName );
        archive& boost::serialization::make_nvp( "filepath", m_filePath );
        archive& boost::serialization::make_nvp( "flags", m_cppFlags );
        archive& boost::serialization::make_nvp( "defines", m_cppDefines );
        archive& boost::serialization::make_nvp( "srcDir", m_srcDir );
        archive& boost::serialization::make_nvp( "buildDir", m_buildDir );
        archive& boost::serialization::make_nvp( "sourceFiles", m_sourceFiles );
        archive& boost::serialization::make_nvp( "dependencyFiles", m_dependencyFiles );
        archive& boost::serialization::make_nvp( "includeDirectories", m_includeDirectories );
    }

private:
    ComponentType              m_componentType;
    std::string                m_strName;
    boost::filesystem::path    m_filePath;
    std::vector< std::string > m_cppFlags;
    std::vector< std::string > m_cppDefines;
    boost::filesystem::path    m_srcDir;
    boost::filesystem::path    m_buildDir;
    PathArray                  m_sourceFiles;
    PathArray                  m_dependencyFiles;
    PathArray                  m_includeDirectories;
};

} // namespace mega::io

#endif // IO_SOURCETREE_25_MARCH_2022
