
#include "common/file.hpp"
#include "common/hash.hpp"
#include "common/stash.hpp"
#include "common/assert_verify.hpp"

#include <boost/program_options.hpp>

#include <iostream>

int main( int argc, const char* argv[] )
{
    namespace po = boost::program_options;
    try
    {
        bool bHelp        = false;
        bool bGeneralWait = false;

        std::vector< std::string > sourceFiles;
        std::string                libDir, templatesDir, outputAPIDir, outputSrcDir, dataDir, stashDir;

        po::options_description commandOptions( " Commands" );
        {
            commandOptions.add_options()
                // clang-format off
            ( "help",   po::bool_switch( &bHelp ),                      "Print command line help info." )
            ( "wait",   po::bool_switch( &bGeneralWait ),               "Wait at startup for attaching a debugger" )

            ( "lib",        po::value< std::string >( &libDir ),        "Database library directory" )
            ( "templates",  po::value< std::string >( &templatesDir ),  "Database templates directory" )
            ( "api",        po::value< std::string >( &outputAPIDir ),  "Output folder to generate API" )
            ( "src",        po::value< std::string >( &outputSrcDir ),  "Output folder to generate source" )
            ( "stash",      po::value< std::string >( &stashDir ),      "Stash directory" )
            ( "data",       po::value< std::string >( &dataDir ),       "Directory for intermediate data" )

            ( "input",  po::value< std::vector< std::string > >( &sourceFiles ), "Input source file" )
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

