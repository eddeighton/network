
#include "meta/configuration.hpp"

#include "vocab/meta/configuration.hpp"

#include "pipeline/version.hpp"
#include "pipeline/configuration.hpp"
#include "pipeline/pipeline.hpp"

#include "meta/db/database/manifest.hxx"
#include "meta/db/database/component_info.hpp"
#include "meta/db/database/directories.hpp"

#include "common/file.hpp"
#include "common/hash.hpp"
#include "common/stash.hpp"
#include "common/assert_verify.hpp"

#include <boost/program_options.hpp>

#include <iostream>

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

int main( int argc, const char* argv[] )
{
    namespace po = boost::program_options;
    try
    {
        bool bHelp        = false;
        bool bGeneralWait = false;

        std::string             projectName;
        boost::filesystem::path srcDir, buildDir, installDir, templatesDir, stashDir, metaPipelinePath;

        po::options_description commandOptions( " Commands" );
        {
            commandOptions.add_options()
                // clang-format off
            ( "mega_project",      po::value< std::string >( &projectName ),                    "Mega Project Name" )
            ( "src_dir",           po::value< boost::filesystem::path >( &srcDir ),             "Root source directory" )
            ( "build_dir",         po::value< boost::filesystem::path >( &buildDir ),           "Root build directory" )
            ( "install_dir",       po::value< boost::filesystem::path >( &installDir ),         "Installation directory" )
            ( "stash_dir",         po::value< boost::filesystem::path >( &stashDir ),           "Stash directory" )
            ( "templates",         po::value< boost::filesystem::path >( &templatesDir ),       "Inja Templates directory" )
            ( "pipeline",          po::value< boost::filesystem::path >( &metaPipelinePath ),   "Meta pipeline path" )
            ;
            // clang-format on
        }

        po::positional_options_description p;
        p.add( "input", -1 );

        po::variables_map vm;
        po::store( po::command_line_parser( argc, argv ).options( commandOptions ).positional( p ).run(), vm );
        po::notify( vm );

        if( bHelp )
        {
            std::cout << commandOptions << "\n";
        }
        else
        {
            //
            mega::Version version;
            
            const mega::io::Directories directories{ srcDir, buildDir, installDir, templatesDir };

            std::vector< mega::io::ComponentInfo > componentInfos;

            // create the manifest
            const mega::io::MetaEnvironment  environment( directories );
            const mega::io::Manifest         manifest( environment, srcDir, componentInfos );

            {
                // save the manifest
                const mega::io::manifestFilePath projectManifestPath = environment.project_manifest();
                manifest.save_temp( environment, projectManifestPath );
                environment.temp_to_real( projectManifestPath );
            }


            // clang-format off
            const mega::meta::Configuration config =
            {
                metaPipelinePath.string(),
                version,

                projectName,
                componentInfos,

                directories,
                manifest
            };

            // clang-format on
            mega::pipeline::Configuration pipelineConfiguration =
                mega::meta::makePipelineConfiguration( config );

            mega::pipeline::PipelineResult pipelineResult =
                mega::pipeline::runPipelineLocally(
                    stashDir,
                    pipelineConfiguration,
                    std::cout );


        }
    }
    catch( boost::program_options::error& e )
    {
        std::cout << "Invalid input. " << e.what() << "\nType '--help' for options" << std::endl;
        return 1;
    }
    catch( std::exception& e )
    {
        std::cout << "Error: " << e.what() << std::endl;
        return 1;
    }
    catch( ... )
    {
        std::cout << "Unknown error.\n" << std::endl;
        return 1;
    }

    return 0;
}

