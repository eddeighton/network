

#pragma once

#include "meta/db/database/manifest.hxx"
#include "meta/db/database/component_info.hpp"
#include "meta/db/database/directories.hpp"

#include "common/string.hpp"

namespace mega::io
{
    class MetaEnvironment : public mega::io::Environment
    {
    protected:
        using Path = boost::filesystem::path;

        const Directories& m_directories;
        const Path         m_tempDir;

        Path tempDir() const
        {
            Path tempDir = boost::filesystem::temp_directory_path() / "megaenv" / common::uuid();
            boost::filesystem::create_directories( tempDir );
            return tempDir;
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

        boost::filesystem::path toPath( const SourceFilePath& key ) const { return m_directories.srcDir / key.path(); }
        boost::filesystem::path toPath( const BuildFilePath& key ) const { return m_directories.buildDir / key.path(); }
    public:
        MetaEnvironment( const mega::io::Directories& directories )
            : m_directories( directories )
            , m_tempDir( tempDir() )
        {
        }

        ////////////////////////////////////////////////////
        ////////////////////////////////////////////////////
        // FileSystem
        virtual bool exists( const BuildFilePath& filePath ) const
        {
            return boost::filesystem::exists( toPath( filePath ) );
        }

        virtual std::unique_ptr< std::istream > read( const BuildFilePath& filePath ) const
        {
            return boost::filesystem::createBinaryInputFileStream( toPath( filePath ) );
        }
        virtual std::unique_ptr< std::ostream > write_temp( const BuildFilePath&     filePath,
                                                            boost::filesystem::path& tempFilePath ) const
        {
            tempFilePath = m_tempDir / filePath.path();
            return boost::filesystem::createBinaryOutputFileStream( tempFilePath );
        }
        virtual void temp_to_real( const BuildFilePath& filePath ) const
        {
            copyToTargetPath( m_tempDir / filePath.path(), toPath( filePath ) );
        }

        virtual std::unique_ptr< std::istream > read( const SourceFilePath& filePath ) const
        {
            return boost::filesystem::createBinaryInputFileStream( toPath( filePath ) );
        }
        virtual std::unique_ptr< std::ostream > write_temp( const SourceFilePath&    filePath,
                                                            boost::filesystem::path& tempFilePath ) const
        {
            tempFilePath = m_tempDir / filePath.path();
            return boost::filesystem::createBinaryOutputFileStream( tempFilePath );
        }
        virtual void temp_to_real( const SourceFilePath& filePath ) const
        {
            copyToTargetPath( m_tempDir / filePath.path(), toPath( filePath ) );
        }
    };
}


