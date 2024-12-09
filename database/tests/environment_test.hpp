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

#ifndef ENVIRONMENT_TEST_21_OCT_2023
#define ENVIRONMENT_TEST_21_OCT_2023

#include "database/directories.hpp"

#include "common/file.hpp"

#include <boost/filesystem/operations.hpp>

#include "common/string.hpp"

namespace mega::io
{

template < class TBaseEnvironment >
class TestEnvironment : public TBaseEnvironment
{
protected:
    using Path = boost::filesystem::path;

    const Directories& m_directories;
    const Path         m_tempDir;

    boost::filesystem::path toPath( const SourceFilePath& key ) const { return m_directories.srcDir / key.path(); }
    boost::filesystem::path toPath( const BuildFilePath& key ) const { return m_directories.buildDir / key.path(); }

    Path tempDir() const
    {
        Path tempDir = boost::filesystem::temp_directory_path() / "megaenv" / common::uuid();
        boost::filesystem::create_directories( tempDir );
        return tempDir;
    }

public:
    TestEnvironment( const Directories& directories )
        : m_directories( directories )
        , m_tempDir( tempDir() )
    {
    }

    const Path& srcDir() const { return m_directories.srcDir; }
    const Path& buildDir() const { return m_directories.buildDir; }

    template < typename TFrom, typename TTo >
    void matchFileTime( const TFrom& from, const TTo& to ) const
    {
        boost::filesystem::last_write_time( toPath( to ), boost::filesystem::last_write_time( toPath( from ) ) );
    }

    template < typename TFilePathType >
    bool exists( const TFilePathType& filePath ) const
    {
        return boost::filesystem::exists( toPath( filePath ) );
    }

    template < typename TFilePathType >
    std::unique_ptr< boost::filesystem::ofstream > write( const TFilePathType& filePath ) const
    {
        return boost::filesystem::createNewFileStream( toPath( filePath ) );
    }

    void copyToTargetPath( const boost::filesystem::path& from, const boost::filesystem::path& to ) const
    {
        VERIFY_RTE_MSG( boost::filesystem::exists( from ), "Failed to locate file: " << from.string() );
        if( boost::filesystem::exists( to ) )
        {
            boost::filesystem::remove( to );
        }
        boost::filesystem::ensureFoldersExist( to );

        // attempt rename - which may fail if temp file is not on same volume as target
        boost::system::error_code ec;
        boost::filesystem::rename( from, to, ec );
        if( ec.failed() )
        {
            boost::filesystem::copy( from, to, boost::filesystem::copy_options::synchronize );
        }
    }

    bool exists( const BuildFilePath& ) const override { return true; }

    std::unique_ptr< std::istream > read( const BuildFilePath& filePath ) const override
    {
        return boost::filesystem::createBinaryInputFileStream( toPath( filePath ) );
    }
    std::unique_ptr< std::ostream > write_temp( const BuildFilePath&     filePath,
                                                boost::filesystem::path& tempFilePath ) const override
    {
        tempFilePath = m_tempDir / filePath.path();
        return boost::filesystem::createBinaryOutputFileStream( tempFilePath );
    }
    void temp_to_real( const BuildFilePath& filePath ) const override
    {
        copyToTargetPath( m_tempDir / filePath.path(), toPath( filePath ) );
    }

    std::unique_ptr< std::istream > read( const SourceFilePath& filePath ) const override
    {
        return boost::filesystem::createBinaryInputFileStream( toPath( filePath ) );
    }
    std::unique_ptr< std::ostream > write_temp( const SourceFilePath&    filePath,
                                                boost::filesystem::path& tempFilePath ) const override
    {
        tempFilePath = m_tempDir / filePath.path();
        return boost::filesystem::createBinaryOutputFileStream( tempFilePath );
    }
    void temp_to_real( const SourceFilePath& filePath ) const override
    {
        copyToTargetPath( m_tempDir / filePath.path(), toPath( filePath ) );
    }
};

} // namespace mega::io

#endif // ENVIRONMENT_TEST_21_OCT_2023
