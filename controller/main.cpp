
#include "controller/locator.hpp"

#include "common/process.hpp"

#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>

#include <iostream>
#include <filesystem>

int main( int argc, const char* argv[] )
{
    std::string strInput;
    bool        bReadInput = false;
    bool        bOutput    = false;
    std::string neovimPipe;
   
    boost::filesystem::path logDir             = boost::filesystem::current_path();
    std::string             strConsoleLogLevel = "info";
    std::string             strLogFileLevel    = "off";
    std::string             strFilter;

    {
        namespace po = boost::program_options;
        po::variables_map vm;

        po::options_description options( " General" );
        {
            // clang-format off
        options.add_options()
            ( "help,?",                                                         "Produce general or command help message" )

            ( "log_dir",    po::value< boost::filesystem::path >( &logDir ),    "Controller log directory" )
            ( "console",    po::value< std::string >( &strConsoleLogLevel ),    "Console logging level" )
            ( "level",      po::value< std::string >( &strLogFileLevel ),       "Log file logging level" )

            ( "input,i",           po::bool_switch( &bReadInput ),              "Read standard input for paths" )
            ( "output,o",          po::bool_switch( &bOutput ),                 "Write paths to output" )
            ( "paths,p",           po::value< std::string >( &strInput ),       "Paths to process via command line" )
            ( "neovim,n",          po::value< std::string >( &neovimPipe ),     "Neovim Pipe to send paths commands to" )
            ( "filter,f",          po::value< std::string >( &strFilter ),         "Filter specified string from filepath i.e. hostname" )
            ;
            // clang-format on
        }

        po::positional_options_description p;
        p.add( "file", -1 );

        po::parsed_options parsedOptions
            = po::command_line_parser( argc, argv ).options( options ).positional( p ).run();
        po::store( parsedOptions, vm );
        po::notify( vm );

        if( vm.count( "help" ) )
        {
            std::cout << options << "\n";
            return 0;
        }
    }
    
    // auto log = logging::configureLog(
    //     logging::Log::Config{ logDir, "controller", logging::fromStr( strConsoleLogLevel ),
    //     logging::fromStr( strLogFileLevel ), false } );

    auto process = [ & ]( const Controller::Path& p )
    {
        std::ostringstream oPath;
        p.to_path( oPath, strFilter );
        const std::filesystem::path filepath( 
            std::filesystem::absolute(oPath.str()));

        //if( !std::filesystem::exists( filepath ) )
        //{
            // Must allow file to not exist because may not be in container
            // return;
        //}
 
        // spdlog::info( "Controller cmd: {}", filepath.string() );

        if( bOutput )
        {
            std::cout << p << "\n";
        }
        if( !neovimPipe.empty() )
        {
            // load path to neovim
            std::ostringstream osCmd;
            osCmd << "nvim --server " << neovimPipe << " --remote-send \"<C-\\><C-N>:find ";

            if( p.m_line )
            {
                osCmd << "+" << p.m_line.value() << " ";
            }
            osCmd << filepath.string();

            osCmd << "<CR>";

            std::string strOut, strError;
            int         r = common::runProcess( osCmd.str(), strOut, strError );
            if( r )
            {
                std::cerr << "Error processing: " << osCmd.str() << std::endl;
                std::cerr << "Error out: " << strOut << std::endl;
                std::cerr << "Error err: " << strError << std::endl;
            }
        }
    };

    if( !strInput.empty() )
    {
        const auto l = Controller::parse< Controller::Line >( strInput );

        for( const Controller::Path& p : l.m_paths )
        {
            process( p );
        }
    }

    if( bReadInput )
    {
        std::string line;
        while( std::cin.good() )
        {
            std::getline( std::cin, line );

            const auto l = Controller::parse< Controller::Line >( line );

            for( const Controller::Path& p : l.m_paths )
            {
                process( p );
            }
        }
    }

    return 0;
}
