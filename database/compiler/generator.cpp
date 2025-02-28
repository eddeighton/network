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



#include "generator.hpp"

#include "common/file.hpp"
#include "common/assert_verify.hpp"

#include "nlohmann/json.hpp"

#include "inja/inja.hpp"
#include "inja/environment.hpp"
#include "inja/template.hpp"

#include <boost/filesystem/operations.hpp>
#include <fstream>
#include <ios>
#include <iostream>
#include <sstream>

namespace db
{
namespace gen
{
namespace
{
nlohmann::json loadJson( const boost::filesystem::path& filePath )
{
    VERIFY_RTE_MSG( boost::filesystem::exists( filePath ), "Could not locate file: " << filePath.string() );
    std::ifstream file( filePath.string(), std::ios_base::in );
    VERIFY_RTE_MSG( !file.fail(), "Failed to open json file: " << filePath.string() );
    return nlohmann::json::parse( std::istreambuf_iterator< char >( file ), std::istreambuf_iterator< char >() );
}

void renderFile( const boost::filesystem::path& basePath, const std::string& fileName, const std::string& strOutput )
{
    const boost::filesystem::path outputFilePath = basePath.string() / boost::filesystem::path( fileName );
    /*if ( boost::filesystem::exists( outputFilePath ) )
    {
        std::ostringstream os;
        os << basePath.string() << "_old" << fileName;
        const boost::filesystem::path oldFile = os.str();
        if ( boost::filesystem::exists( oldFile ) )
            boost::filesystem::remove( oldFile );
        boost::filesystem::ensureFoldersExist( oldFile );
        boost::filesystem::copy_file( outputFilePath, oldFile, boost::filesystem::copy_options::synchronize );
    }*/
    if ( boost::filesystem::updateFileIfChanged( outputFilePath, strOutput ) )
    {
        // std::cout << "Regenerated: " << outputFilePath.string() << std::endl;
    }
}

} // namespace

void generate( const Environment& env )
{
    for ( boost::filesystem::directory_iterator iter( env.dataDir ); iter != boost::filesystem::directory_iterator();
          ++iter )
    {
        const boost::filesystem::path& filePath = *iter;
        if ( !boost::filesystem::is_directory( filePath ) )
        {
            if ( filePath.extension() == ".json" && filePath.filename() != "data.json"
                 && filePath.filename() != "stages.json" )
            {
                // view.hxx
                try
                {
                    std::string strOutput;
                    {
                        std::ostringstream osOutput;
                        inja::Environment  injaEnv( env.injaDir.string(), env.apiDir.string() );
                        injaEnv.set_trim_blocks( true );
                        const boost::filesystem::path jsonFile = env.dataDir / filePath.filename();
                        const auto                    data     = loadJson( jsonFile );
                        inja::Template                tmp      = injaEnv.parse_template( "/view.hxx.jinja" );
                        injaEnv.render_to( osOutput, tmp, data );
                        strOutput = osOutput.str();
                    }
                    std::ostringstream osTargetFile;
                    osTargetFile << "/" << filePath.filename().replace_extension( ".hxx" ).string();
                    renderFile( env.apiDir, osTargetFile.str(), strOutput );
                }
                catch ( std::exception& ex )
                {
                    THROW_RTE( "Error processing template: "
                               << "view.hxx.jinja"
                               << " with data: " << filePath.string() << " Error: " << ex.what() );
                }

                // view.cxx
                try
                {
                    std::string strOutput;
                    {
                        std::ostringstream osOutput;
                        inja::Environment  injaEnv( env.injaDir.string(), env.srcDir.string() );
                        injaEnv.set_trim_blocks( true );
                        const boost::filesystem::path jsonFile = env.dataDir / filePath.filename();
                        const auto                    data     = loadJson( jsonFile );
                        inja::Template                tmp      = injaEnv.parse_template( "/view.cxx.jinja" );
                        injaEnv.render_to( osOutput, tmp, data );
                        strOutput = osOutput.str();
                    }
                    std::ostringstream osTargetFile;
                    osTargetFile << "/" << filePath.filename().replace_extension( ".cxx" ).string();
                    renderFile( env.srcDir, osTargetFile.str(), strOutput );
                }
                catch ( std::exception& ex )
                {
                    THROW_RTE( "Error processing template: "
                               << "view.cxx.jinja"
                               << " with data: " << filePath.string() << " Error: " << ex.what() );
                }
            }
        }
    }

    // clang-format off
    std::vector< std::pair< std::string, std::string > > filenames = 
    {
        { "data"        , "data.json" },
        { "environment" , "stages.json" },    
        { "manifest"    , "stages.json" },    
        { "file_info"   , "stages.json" }   
    };
    // clang-format on

    for ( const std::pair< std::string, std::string >& names : filenames )
    {
        const boost::filesystem::path jsonFile = env.dataDir / names.second;
        //if( names.second != "data.json" )
        {
            try
            {
                std::string strOutput;
                {
                    std::ostringstream osOutput;
                    inja::Environment  injaEnv( env.injaDir.string(), env.apiDir.string() );
                    injaEnv.set_trim_blocks( true );
                    const auto data = loadJson( jsonFile );

                    inja::Template headerTemplate = injaEnv.parse_template( "/" + names.first + ".hxx.jinja" );
                    injaEnv.render_to( osOutput, headerTemplate, data );
                    strOutput = osOutput.str();
                }

                std::ostringstream osTargetFile;
                osTargetFile << "/" + names.first + ".hxx";
                renderFile( env.apiDir, osTargetFile.str(), strOutput );
            }
            catch ( std::exception& ex )
            {
                THROW_RTE( "Error processing template: " << names.first << ".hxx.jinja"
                                                         << " with data: " << jsonFile.string()
                                                         << " Error: " << ex.what() );
            }
        }
        {
            try
            {
                std::string strOutput;
                {
                    std::ostringstream osOutput;
                    inja::Environment  injaEnv( env.injaDir.string(), env.srcDir.string() );
                    injaEnv.set_trim_blocks( true );
                    const auto data = loadJson( jsonFile );

                    inja::Template sourceTemplate = injaEnv.parse_template( "/" + names.first + ".cxx.jinja" );
                    injaEnv.render_to( osOutput, sourceTemplate, data );
                    strOutput = osOutput.str();
                }

                std::ostringstream osTargetFile;
                osTargetFile << "/" + names.first + ".cxx";
                renderFile( env.srcDir, osTargetFile.str(), strOutput );
            }
            catch ( std::exception& ex )
            {
                THROW_RTE( "Error processing template: " << names.first << ".cxx.jinja"
                                                         << " with data: " << jsonFile.string()
                                                         << " Error: " << ex.what() );
            }
        }
    }
}
} // namespace gen
} // namespace db
