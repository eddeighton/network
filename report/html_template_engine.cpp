
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

#include "report/html_template_engine.hpp"

#include "common/string.hpp"
#include "common/assert_verify.hpp"
#include "common/file.hpp"
#include "common/process.hpp"

#include "inja/inja.hpp"
#include "inja/environment.hpp"
#include "inja/template.hpp"

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>

namespace report
{
namespace
{

// clang-format off
static std::array< std::string, HTMLTemplateEngine::TOTAL_TEMPLATE_TYPES > g_defaultTemplates = {

/////////////////////////////////////////////////////////////////
// report template
R"TEMPLATE(

<html>

<head>
<style>
table
{
    font-size:14px;
    font-family:monospace;
    border-collapse: collapse;
}
th
{
    border: 2px solid darkblue;
    border-collapse: collapse;
}
td 
{
    border: 1px solid darkblue;
    border-collapse: collapse;
}
.multiline_default
{
    white-space:pre;
    font-size:14px;
    font-family:monospace;
    background-color:#b0c0c0;
}
.branch_default
{
    font-size:14px;
    font-family:monospace;
    background-color:#cfdfdf;
}
</style>
</head>

<body>

<script type="text/javascript">

    currentReportType = "home"
    
    stateMachine = function() 
    {
        const queryString = window.location.search
        const urlParams = new URLSearchParams(queryString);

        if( window.location.hash.length === 0 )
        {
            // no fragment - nothing to do
            console.log( "No fragment nothing to do" )
        }
        else
        {
            hash = window.location.hash.substring( 1 )
            const element = document.getElementById( hash );
            if( element )
            {
                element.scrollIntoView({ behavior: "smooth", block: "center", inline: "center" });
                console.log( "Scrolled to element: " + hash )
            }
            else
            {I
                console.log( "Could not locate element: " + hash )
            }
        }

        if( urlParams.has( "report" ) )
        {
            currentReportType = urlParams.get( "report" )
            console.log( "Report Type is: " + currentReportType )
        }
    }

    window.onrefresh    = stateMachine
    window.onload       = stateMachine
    
    function keyCodeToReportType( keyCode )
    {
        if( keyCode == 0 )
        {
            return ""
        }
{% for report in reports %}
        else if( keyCode == {{ report.key_code }} )
        {
            return "{{ report.name }}";
        }
{% endfor %}
        return ""
    }

    document.onkeypress = function (e) 
    {
        e = e || window.event;
        lastKeyPressed = e.keyCode
        console.log( "onkeypress= " + lastKeyPressed )
        currentReportType = keyCodeToReportType( lastKeyPressed );
        console.log( "Report Type is: " + currentReportType )
    };

    function navigateTo( path, params, fragment ) 
    {
        if( params )
        {
            newLocation = path + "?" + params + "&report=" + currentReportType
        }
        else
        {
            newLocation = path + "?report=" + currentReportType
        }

        if( fragment )
        {
            newLocation = newLocation + "#" + fragment
        }

        console.log( "href= " + newLocation + "\n" )

        window.location.href = newLocation
    }

</script>

<div style="position:relative;min-width:960px">
    <table style="position: absolute;right:0;top:0">
        <tr><th>Report</th><th>Shortcut Key</th></tr>
        {% for report in reports %}
        <tr><td>{{ report.name }}</td><td>{{ report.key_char }}</td></tr>
        {% endfor %}
    </table>
</div>

{{ body }}
</body>

</html>

)TEMPLATE",

/////////////////////////////////////////////////////////////////
// multiline template
R"TEMPLATE(<div class="{{style}}" style="color:{{ colour }};background-color:{{ background_colour }};" {% if has_bookmark %}id="{{ bookmark }}"{% endif %}>{% if has_link %}<a href="{{ link }}">{% endif %}{% for element in elements %}{{ element }}{% endfor %}{% if has_link %}</a>{% endif %}</div>)TEMPLATE",

/////////////////////////////////////////////////////////////////
// branch template
R"TEMPLATE(
<div class="{{style}}" {% if has_bookmark %}id="{{ bookmark }}"{% endif %}>{% for element in label %}{{ element }}{% endfor %}</div>
<ul class="{{style}}" >
{% for element in elements %}
    <li>{{ element }}</li>
{% endfor %}
</ul>

)TEMPLATE",

/////////////////////////////////////////////////////////////////
// table template
R"TEMPLATE(
<table>
{% if headings %}
<tr>
{% for heading in headings %}
<th>{{ heading }}</th>
{% endfor %}
</tr>
{% endif %}
{% for row in rows %}
<tr>
{% for value in row.values %}
<td>{{ value }}</td>
{% endfor %}
</tr>
{% endfor %}
</table>)TEMPLATE",

/////////////////////////////////////////////////////////////////
// plot template
R"TEMPLATE(

set terminal svg size 600,400 dynamic enhanced font 'arial,10' mousing name "plot" dashlength 1.0 

set output "plot.svg"

set key fixed left top vertical Right noreverse enhanced autotitle box lt black linewidth 1.000 dashtype solid

set xlabel "Execution Date" 
set ylabel "Benchmark Time ( ns )" 

set title "{% for heading in headings %}{{ heading }} {% endfor %}" 
set title font ",20" textcolor lt -1 norotate

set boxwidth 0.75 relative

set style fill transparent solid 0.5

set bars linecolor 'blue' linewidth 1.0 dashtype '.'

plot 'plot.dat' with boxes fc 'blue', \
    '' with labels hypertext



)TEMPLATE",

/////////////////////////////////////////////////////////////////
// graph template
R"TEMPLATE(digraph
{
    rankdir       = {{ rank_direction }};
    ratio         = auto;
    concentrate   = True;

    node 
    [ 
        shape       = rectangle 
        style       = solid
        penwidth    = 1
        fontname    = monospace
        fontsize    = 10
        fontcolor   = black
        margin      = 0 
        width       = 0 
        height      = 0
    ];

    edge 
    [ 
        fontname  = monospace
        fontsize  = 10
        fontcolor = black
    ];

    graph 
    [
        splines   = true 
        overlap   = false
    ]

{% for node in nodes %}
    "{{ node.name }}" 
    [
{% if node.has_url %}
        URL         = "{{ node.url }}"
{% endif %}
        label       = 
        <
            <table 
                border      = "{{ node.border_width }}" 
                color       = "{{ node.colour }}" 
                cellborder  = "1"
                cellpadding = "1"
                cellspacing = "0"
                bgcolor     = "{{ node.bgcolour }}"
            >
            {% for row in node.rows %}
            <tr>
            {% for value in row.values %}
                {{ value }}
            {% endfor %}
            </tr>
            {% endfor %}
            </table>
        >
    ];
{% endfor %}

{% for subgraph in subgraphs %}
    subgraph {{ subgraph.name }}
    {
        style       = dashed
        penwidth    = 2
{% if subgraph.has_url %}
        URL         = "{{ subgraph.url }}"
{% endif %}
{% if subgraph.has_label %}
        label       = 
        <
            <table 
                border      = "1" 
                cellborder  = "1" 
                cellpadding = "0"
                bgcolor     = "{{ subgraph.colour }}"
            >
            {% for row in subgraph.rows %}
            <tr>
            {% for value in row.values %}
                {{ value }}
            {% endfor %}
            </tr>
            {% endfor %}
            </table>
        >
{% endif %}

{% for node in subgraph.nodes %}
        {{ node }};
{% endfor %}
    }
{% endfor %}

{% for edge in edges %}
    {{ edge.from }} -> {{ edge.to }}
    [
{% if edge.has_label %}
        label       = 
        <
            <table 
                border      = "1" 
                cellborder  = "1" 
                cellpadding = "0"
            >
            <tr>
            {% for value in edge.label %}
                {{ value }}
            {% endfor %}
            </tr>
            </table>
        >
{% endif %}
        penwidth    = "{{ edge.line_width }}"
        color       = "{{ edge.colour }}"
        style       = "{{ edge.style }}"
        constraint  = {{ edge.constraint }}
    ]

{% endfor %}
})TEMPLATE"

};
// clang-format on

} // namespace

HTMLTemplateEngine::HTMLTemplateEngine( bool bClearTempFiles )
    : m_pEnvironment( std::make_unique< inja::Environment >() )
    , m_templateNames{ "report.jinja", "multiline.jinja", "branch.jinja", "table.jinja", "plot.jinja", "graph.jinja" }
    , m_tempFolder( boost::filesystem::temp_directory_path() / "graphs" / common::uuid() )
    , m_bClearTempFiles( bClearTempFiles )
{
    boost::filesystem::create_directories( m_tempFolder );
    VERIFY_RTE_MSG(
        boost::filesystem::exists( m_tempFolder ), "Failed to create temporary folder: " << m_tempFolder.string() );

    m_pEnvironment->set_trim_blocks( true );
    for( unsigned int i = 0U; i != TOTAL_TEMPLATE_TYPES; ++i )
    {
        const auto templateType = static_cast< TemplateType >( i );
        // use default templates instead of loading from disk
        m_templates[ templateType ]
            = std::make_unique< inja::Template >( m_pEnvironment->parse( g_defaultTemplates[ i ] ) );
    }
}

HTMLTemplateEngine::HTMLTemplateEngine( const boost::filesystem::path& templateDir, bool bClearTempFiles )
    : m_pEnvironment( std::make_unique< inja::Environment >() )
    , m_templateNames{ "report.jinja", "multiline.jinja", "branch.jinja", "table.jinja", "plot.jinja", "graph.jinja" }
    , m_tempFolder( boost::filesystem::temp_directory_path() / "graphs" / common::uuid() )
    , m_bClearTempFiles( bClearTempFiles )
{
    boost::filesystem::create_directories( m_tempFolder );
    VERIFY_RTE_MSG(
        boost::filesystem::exists( m_tempFolder ), "Failed to create temporary folder: " << m_tempFolder.string() );

    m_pEnvironment->set_trim_blocks( true );
    for( unsigned int i = 0U; i != TOTAL_TEMPLATE_TYPES; ++i )
    {
        const auto templateType = static_cast< TemplateType >( i );
        auto       templatePath = templateDir / m_templateNames[ templateType ];
        VERIFY_RTE_MSG(
            boost::filesystem::exists( templatePath ), "Failed to locate report template: " << templatePath.string() );

        m_templates[ templateType ]
            = std::make_unique< inja::Template >( m_pEnvironment->parse_template( templatePath.string() ) );
    }
}

HTMLTemplateEngine::~HTMLTemplateEngine()
{
    if( m_bClearTempFiles )
    {
        boost::filesystem::remove_all( m_tempFolder );
    }
}

void HTMLTemplateEngine::renderTemplate( const nlohmann::json& data, TemplateType templateType, std::ostream& os )
{
    try
    {
        m_pEnvironment->render_to( os, *m_templates[ templateType ], data );
    }
    catch( ::inja::RenderError& ex )
    {
        THROW_RTE( "Inja Exception rendering template: " << m_templateNames[ templateType ]
                                                         << " error: " << ex.what() );
    }
    catch( std::exception& ex )
    {
        THROW_RTE( "Exception rendering template: " << m_templateNames[ templateType ] << " error: " << ex.what() );
    }
}

void HTMLTemplateEngine::renderPlot( const nlohmann::json& data, std::ostream& os )
{
    // generate the data file
    {
        boost::filesystem::path tempDataFile = m_tempFolder / "plot.dat";
        auto                    pDataFile    = boost::filesystem::createNewFileStream( tempDataFile );
        for( const auto& p : data[ "points" ] )
        {
            int iCount = 0;
            for( const auto& value : p[ "values" ] )
            {
                if( iCount < 2 )
                {
                    *pDataFile << value.get< std::string >() << " ";
                }
                else
                {
                    *pDataFile << '\"' << value.get< std::string >() << "\" ";
                }
                ++iCount;
            }
            *pDataFile << "\n";
        }
    }

    boost::filesystem::path tempGNUPlotFile = m_tempFolder / "plot.txt";
    {
        // render the template
        std::ostringstream osGNUPlot;
        renderTemplate( data, ePlot, osGNUPlot );
        // write the temporary file
        {
            auto pTempFile = boost::filesystem::createNewFileStream( tempGNUPlotFile );
            *pTempFile << osGNUPlot.str();
        }
    }

    // run gnuplot
    {
        auto currentDir = boost::filesystem::current_path();
        boost::filesystem::current_path( m_tempFolder );

        try
        {
            std::ostringstream osCmd;
            osCmd << "gnuplot " << tempGNUPlotFile.string();

            std::string strOutput, strError;
            const int   iExitCode = common::runProcess( osCmd.str(), strOutput, strError );
            VERIFY_RTE_MSG( iExitCode == EXIT_SUCCESS, "gnuplot failed with error: " << strError );
            VERIFY_RTE_MSG( strError.empty(), "gnuplot failed with error: " << strError );
            VERIFY_RTE_MSG( strOutput.empty(), "gnuplot failed with output: " << strOutput );
        }
        catch( std::exception& ex )
        {
            boost::filesystem::current_path( currentDir );
            throw;
        }

        boost::filesystem::current_path( currentDir );
    }

    // load the resultant svg file
    std::string strSVG;
    {
        boost::filesystem::path tempSVGFile = m_tempFolder / "plot.svg";
        boost::filesystem::loadAsciiFile( tempSVGFile, strSVG );
    }
    os << strSVG;
}

void HTMLTemplateEngine::renderGraph( const nlohmann::json& data, std::ostream& os )
{
    std::ostringstream osDot;
    renderTemplate( data, eGraph, osDot );

    boost::filesystem::path tempDotFile = m_tempFolder / "temp.dot";
    boost::filesystem::path tempSVGFile = m_tempFolder / "temp.svg";

    // write the temporary file
    {
        auto pTempFile = boost::filesystem::createNewFileStream( tempDotFile );
        *pTempFile << osDot.str();
    }

    // run graphviz
    {
        std::ostringstream osCmd;
        osCmd << "dot -Tsvg -o" << tempSVGFile.string() << " " << tempDotFile.string();

        std::string strOutput, strError;
        common::runProcess( osCmd.str(), strOutput, strError );
        VERIFY_RTE_MSG( strError.empty(), "Graphviz failed with error: " << strError );
        VERIFY_RTE_MSG( strOutput.empty(), "Graphviz failed with output: " << strOutput );
    }

    // deal with annoying graphviz behaviour with how id is generated where it doesnt work as bookmark

    std::string str;
    {
        boost::filesystem::loadAsciiFile( tempSVGFile, str );

        static const std::string strSearch  = "<g id=\"a_";
        static const std::string strReplace = "<text ";

        using Iter = std::string::iterator;

        for( Iter i = str.begin(), iEnd = str.end(); i != iEnd;
             i = std::search( i, iEnd, strSearch.begin(), strSearch.end() ) )
        {
            Iter r = std::search( i, iEnd, strReplace.begin(), strReplace.end() );
            if( r != iEnd )
            {
                Iter idStart    = i + 2; // NOTE: include the space before the id=
                int  quoteCount = 0;

                // find the end of the id="something" by counting the quotes
                Iter idEnd = idStart;
                for( ; ( idEnd != iEnd ) && ( quoteCount != 2 ); )
                {
                    if( *idEnd == '\"' )
                    {
                        ++quoteCount;
                    }
                    ++idEnd;
                }

                // want to move the id string to 5 chars after r ( ignore the space )
                Iter newID = r + 6;

                // move ( idStart to idEnd ) to newID
                std::string strTemp( idStart, idEnd );
                boost::replace_all( strTemp, "\"a_", "  \"" );

                auto iNewIDStart = std::copy( idEnd, newID, idStart );
                std::copy( strTemp.begin(), strTemp.end(), iNewIDStart );
            }
        }
    }
    os << str;
}

void HTMLTemplateEngine::render( TemplateType templateType, const nlohmann::json& data, std::ostream& os )
{
    switch( templateType )
    {
        case eReport:
        {
            renderTemplate( data, eReport, os );
        }
        break;
        case eMultiLine:
        {
            renderTemplate( data, eMultiLine, os );
        }
        break;
        case eBranch:
        {
            renderTemplate( data, eBranch, os );
        }
        break;
        case eTable:
        {
            renderTemplate( data, eTable, os );
        }
        break;
        case ePlot:
        {
            renderPlot( data, os );
        }
        break;
        case eGraph:
        {
            renderGraph( data, os );
        }
        break;
        default:
        case TOTAL_TEMPLATE_TYPES:
        {
            THROW_RTE( "Unknown template type" );
        }
        break;
    }
}

} // namespace report
