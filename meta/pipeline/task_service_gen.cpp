



#include "meta/db/database/AnalysisStage.hxx"
#include "meta/environment.hpp"
#include "meta/task.hpp"

#include "common/file.hpp"
#include "common/assert_verify.hpp"


#include "nlohmann/json.hpp"

#include "inja/inja.hpp"
#include "inja/environment.hpp"
#include "inja/template.hpp"

namespace mega::meta
{

static void renderTemplate(
        const boost::filesystem::path& templateFilePath,
        const boost::filesystem::path& outputFilePath,
        const boost::filesystem::path& templateDir,
        const nlohmann::json& data )
{
    try
    {
        std::string strOutput;
        {
            std::ostringstream osOutput;
            inja::Environment  injaEnv( templateDir.string() );
            injaEnv.set_trim_blocks( true );
            std::ostringstream osAddSlash;
            osAddSlash << '/' << templateFilePath.string();
            inja::Template tmp = injaEnv.parse_template( osAddSlash.str() );
            injaEnv.render_to( osOutput, tmp, data );
            strOutput = osOutput.str();
        }
        boost::filesystem::updateFileIfChanged(outputFilePath, strOutput);
    }
    catch (std::exception& ex)
    {
        THROW_RTE( "Error processing template: "
                   << templateFilePath.string()
                   << " generating: " << outputFilePath.string() 
                   << " Error: " << ex.what() );
    }
}

using namespace AnalysisStage;

void task_service_gen(TaskDependencies& dependencies)
{
    using namespace std::string_literals;
    using namespace AnalysisStage::Service;

    auto src = dependencies.m_environment.project_manifest();
    Database database( dependencies.m_environment, src, true );

    nlohmann::json data(
        {
            { "includes", nlohmann::json::array() },
            { "interfaces", nlohmann::json::array() }
        }
    );

    auto qualifiedTypeToString = []( const QualifiedType* pType ) -> std::string
    {
        std::ostringstream os;
        if( pType->get_is_const() )
        {
            os << "const ";
        }
        os << pType->get_name();
        if( pType->get_is_pointer() )
        {
            os << "*";
        }
        if( pType->get_is_reference() )
        {
            if( pType->get_is_rvalue() )
            {
                os << "&&";
            }
            else
            {
                os << "&";
            }
        }
        return os.str();
    };
 
    std::set< std::string > includes;

    for( auto pInterface : database.many< Interface >(src) )
    {
        includes.insert(pInterface->get_include_path());

        nlohmann::json interface(
            {
                { "type_name", pInterface->get_type_name() },
                { "full_type_name", pInterface->get_full_type_name() },
                { "functions", nlohmann::json::array() },
                { "namespaces", pInterface->get_namespaces() }
            }
        );
        for( auto pFunction : pInterface->get_functions() )
        {
            nlohmann::json function(
                {
                    { "name", pFunction->get_name() },
                    { "return_type", qualifiedTypeToString( pFunction->get_return_type() ) },
                    { "parameters", nlohmann::json::array() }
                }
            );
            for( auto pParam : pFunction->get_parameters() )
            {
                nlohmann::json parameter(
                    {
                        { "name", pFunction->get_name() },
                        { "type", qualifiedTypeToString( pParam->get_qualified_type() ) }
                    }
                );
                function[ "parameters" ].push_back( parameter );
            }
            interface[ "functions" ].push_back( function );
        }
        data[ "interfaces" ].push_back( interface );
    }

    for( const auto& include : includes )
    {
        data[ "includes" ].push_back( include );
    }
 
    renderTemplate( 
        dependencies.m_environment.getServiceTemplate_registry(),
        dependencies.m_environment.getServiceCodeGen_registry(),
        dependencies.m_environment.getDirectories().templatesDir,
        data );

    renderTemplate( 
        dependencies.m_environment.getServiceTemplate_receiver(),
        dependencies.m_environment.getServiceCodeGen_receiver(),
        dependencies.m_environment.getDirectories().templatesDir,
        data );

    renderTemplate( 
        dependencies.m_environment.getServiceTemplate_proxy_itc(),
        dependencies.m_environment.getServiceCodeGen_proxy_itc(),
        dependencies.m_environment.getDirectories().templatesDir,
        data );

    renderTemplate( 
        dependencies.m_environment.getServiceTemplate_proxy_ipc(),
        dependencies.m_environment.getServiceCodeGen_proxy_ipc(),
        dependencies.m_environment.getDirectories().templatesDir,
        data );
}

}

