

#include "report/report.hpp"
#include "report/renderer_html.hpp"

#include "meta/db/database/AnalysisStage.hxx"
#include "meta/environment.hpp"
#include "meta/task.hpp"

#include "common/file.hpp"
#include "common/assert_verify.hpp"
#include "common/file.hpp"

#include <boost/algorithm/string.hpp>
#include <boost/filesystem/path.hpp>

#include <string>
#include <vector>
#include <sstream>

namespace mega::meta
{

using namespace AnalysisStage;

void task_interface_analysis_report(TaskDependencies& dependencies)
{
    using namespace std::string_literals;
    using namespace AnalysisStage::Service;

    // start( taskProgress, "Task_InterfaceAnalysis" );
    // TODO: implement determinant and stashing

    // task::DeterminantHash determinant( 123 );

    //        { m_toolChain.toolChainHash, m_environment.getBuildHashCode( ... ) } );

    // if( m_environment.restore( compilationFilePath, determinant ) )
    // {
    //     m_environment.setBuildHashCode( compilationFilePath );
    //     cached( taskProgress );
    //     return;
    // 
    //

    auto src = dependencies.m_environment.project_manifest();

    auto qualTypeToStr = []( const QualifiedType* pType ) -> std::string
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
        if( pType->get_is_rvalue() )
        {
            os << "&&";
        }
        else if( pType->get_is_reference() )
        {
            os << "&";
        }
        return os.str();
    };

    // load the database from analysis
    Database database( dependencies.m_environment, src, true );

    using namespace std::string_literals;

    using V = std::variant< bool, std::string >;

    using namespace report;

    using Cont = Container< V >;
    using B    = Branch< V >;
    using L    = Line< V >;
    using M    = Multiline< V >;
    using T    = Table< V >;
    using P    = Plot< V >;
    using G    = Graph< V >;

    B interfaces{ { "Interfaces"s } };
    B factories{ { "Factories"s } };
    B daemons{ { "Daemons"s } };

    for( auto pInterface : database.many< Interface >(src) )
    {
        std::vector< ContainerVector< V > > rows;
        for( auto pFunction : pInterface->get_functions() )
        {
            report::ContainerVector< V > row;
            row.push_back( L{ pFunction->get_name() } );
            row.push_back( L{ qualTypeToStr( pFunction->get_return_type() ) } );

            std::vector< ContainerVector< V > > params;
            for( auto pParam : pFunction->get_parameters() )
            {
                auto pQualifiedType = pParam->get_qualified_type();

                report::ContainerVector< V > param;
                param.push_back( L{ qualTypeToStr( pParam->get_qualified_type() ) } );
                param.push_back( L{ pParam->get_name() } );
                param.push_back( L{ pQualifiedType->get_is_const() } );
                param.push_back( L{ pQualifiedType->get_is_pointer() } );
                param.push_back( L{ pQualifiedType->get_is_reference() } );
                param.push_back( L{ pQualifiedType->get_is_rvalue() } );
                param.push_back( L{ pQualifiedType->get_name() } );
                params.push_back(param);
            }
            row.push_back( T
            {
                { "Type", "Name", "Const", "Pointer", "Reference", "RValue", "Underlying Type" },
                params
            });

            rows.push_back(row);
        }

        auto r = B
        {
            { pInterface->get_full_type_name() },
            { 
                T
                {
                    { "Function Name"s, "Return Type"s, "Parameters"s },
                    rows
                }
            }
        };       

        if( auto pFactory = db_cast< Factory >( pInterface ) )
        {
            std::cout << "Report found factory: " << pInterface->get_full_type_name() << std::endl;
            factories.m_elements.push_back(r);
        }   
        else if( auto pDaemon = db_cast< Daemon >( pInterface ) )
        {
            std::cout << "Report found daemon: " << pInterface->get_full_type_name() << std::endl;
            daemons.m_elements.push_back(r);
        }
        else
        {
            std::cout << "Report found interface: " << pInterface->get_full_type_name() << std::endl;
            interfaces.m_elements.push_back(r);
        }
    }

    Cont report =
        B
        {
            { "Megastructure Service Interface Report"s },
            {
                interfaces,
                factories,
                daemons
            }
        };

    std::ostringstream os;
    renderHTML( report, os );

    auto pFile = boost::filesystem::createNewFileStream( "/src/meta/interface_report.html" );
    *pFile << os.str();

}

}
