




#include "meta/configuration.hpp"
#include "meta/environment.hpp"

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
#include "common/cmake.hpp"

#include <boost/program_options.hpp>

#include <iostream>

int main( int argc, const char* argv[] )
{
    namespace po = boost::program_options;
    try
    {
        bool bHelp        = false;
        bool bGeneralWait = false;

        boost::filesystem::path srcDir, buildDir, templatesDir, stashDir, metaPipelinePath;
        std::string interfaceFilePathsCMakeString;

        po::options_description commandOptions( " Commands" );
        {
            commandOptions.add_options()
                // clang-format off
            ( "help",   po::bool_switch( &bHelp ),                      "Print command line help info." )
            ( "wait",   po::bool_switch( &bGeneralWait ),               "Wait at startup for attaching a debugger" )

            ( "src_dir",           po::value< boost::filesystem::path >( &srcDir ),             "Root source directory" )
            ( "build_dir",         po::value< boost::filesystem::path >( &buildDir ),           "Root build directory" )
            ( "stash_dir",         po::value< boost::filesystem::path >( &stashDir ),           "Stash directory" )
            ( "templates",         po::value< boost::filesystem::path >( &templatesDir ),       "Inja Templates directory" )
            ( "pipeline",          po::value< boost::filesystem::path >( &metaPipelinePath ),   "Meta pipeline path" )
            ( "interfaces",        po::value< std::string >( &interfaceFilePathsCMakeString ),  "Interface file paths" )
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
            // process cmake lists
            const std::vector< boost::filesystem::path > interfaceFilePaths
                = mega::utilities::pathListToFiles(
                        mega::utilities::parseCMakeStringList( interfaceFilePathsCMakeString, " " ) );

            std::cout << "Got interface file paths of: ";
            std::copy( interfaceFilePaths.begin(), interfaceFilePaths.end(),
                    std::ostream_iterator< boost::filesystem::path >( std::cout, " " ) );

            mega::Version version;

            const task::FileHash pipelineHash( metaPipelinePath );
     
            const mega::io::Directories directories{ srcDir, buildDir, buildDir, templatesDir };

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

                pipelineHash,
                componentInfos,

                directories,
                manifest,

                interfaceFilePaths
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

