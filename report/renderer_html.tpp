
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

#ifndef GUARD_2024_March_08_renderer_html
#define GUARD_2024_March_08_renderer_html

#include "common/process.hpp"
#include "common/file.hpp"
#include "common/string.hpp"
#include "common/assert_verify.hpp"

#include <nlohmann/json.hpp>

#include "boost/filesystem.hpp"
#include <boost/algorithm/string.hpp>

#include <algorithm>
#include <string>

namespace report
{

namespace detail
{

inline std::string escapeHTML( std::string data )
{
    using boost::algorithm::replace_all;
    replace_all( data, "&", "&amp;" );
    replace_all( data, "\"", "&quot;" );
    replace_all( data, "\'", "&apos;" );
    replace_all( data, "<", "&lt;" );
    replace_all( data, ">", "&gt;" );
    return data;
}

inline std::string javascriptHREF( const URL& url )
{
    std::ostringstream os;

    os << "javascript:navigateTo( &quot;" << url.encoded_path() << "&quot;,";

    bool bHasReportType = false;
    {
        auto iFind = url.params().find( "report" );
        if( iFind != url.params().end() )
        {
            bHasReportType = true;
        }
    }

    if( ( !url.params().empty() ) && ( !bHasReportType || ( url.params().size() > 1 ) ) )
    {
        if( bHasReportType )
        {
            // remove the report type
            URL temp = url;
            temp.params().erase( "report" );
            os << " &quot;" << temp.encoded_params() << "&quot; ,";
        }
        else
        {
            os << " &quot;" << url.encoded_params() << "&quot; ,";
        }
    }
    else
    {
        os << " &quot;&quot; ,";
    }

    if( url.has_fragment() )
    {
        os << " &quot;" << url.encoded_fragment() << "&quot; )";
    }
    else
    {
        os << " &quot;&quot; )";
    }

    return os.str();
}

template < typename Value >
inline void valueToJSON( HTMLTemplateEngine&, const Value& value, nlohmann::json& data )
{
    std::optional< URL > urlOpt;

    /*if( engine.pLinker )
    {
        urlOpt = engine.pLinker->link( value );
    }*/

    std::ostringstream os;
    if( urlOpt.has_value() )
    {
        os << "<a href=\"" << javascriptHREF( urlOpt.value() ) << "\">" << escapeHTML( toString( value ) ) << "</a>";
    }
    else
    {
        os << escapeHTML( toString( value ) );
    }

    data.push_back( os.str() );
}

template < typename Value >
inline void graphValueToJSON( HTMLTemplateEngine&, const Value& value, nlohmann::json& data )
{
    std::ostringstream os;

    os << "<td";

    std::optional< URL > urlOpt;
    /*if( engine.pLinker )
    {
        urlOpt = engine.pLinker->link( value );
    }*/

    /*if( urlOpt.has_value() )
    {
        os << " href=\"" << javascriptHREF( urlOpt.value() ) << "\"><U>" << escapeHTML( toString( value ) )
           << "</U></td>";
    }
    else*/
    {
        os << ">" << escapeHTML( toString( value ) ) << "</td>";
    }

    data.push_back( os.str() );
}

template < typename Value >
inline void graphValueToJSON( HTMLTemplateEngine&,
                              const Value&                  value,
                              const std::optional< Value >& bookmarkOpt,
                              nlohmann::json&               data )
{
    std::ostringstream os;

    os << "<td";

    std::optional< URL > urlOpt;
    /*if( engine.pLinker )
    {
        urlOpt = engine.pLinker->link( value );
    }*/

    if( bookmarkOpt.has_value() )
    {
        os << " ID=\"" << escapeHTML( toString( bookmarkOpt.value() ) ) << "\"";
        if( !urlOpt.has_value() )
        {
            // ensure href because graphviz will NOT generate ID if no href present in <td>
            URL url;
            url.set_fragment( toString( bookmarkOpt.value() ) );
            os << " href=\"" << javascriptHREF( url ) << "\"";
        }
    }

    /*if( urlOpt.has_value() )
    {
        os << " href=\"" << javascriptHREF( urlOpt.value() ) << "\"><U>" << escapeHTML( toString( value ) )
           << "</U></td>";
    }
    else*/
    {
        os << ">" << escapeHTML( toString( value ) ) << "</td>";
    }

    data.push_back( os.str() );
}

template < typename Value >
inline void
valueVectorToJSON( HTMLTemplateEngine& engine, const ValueVector< Value >& textVector, nlohmann::json& data )
{
    for( const auto& text : textVector )
    {
        valueToJSON( engine, text, data );
    }
}

template < typename T >
inline void addOptionalBookmark( HTMLTemplateEngine&, T& element, nlohmann::json& data )
{
    if( element.m_bookmark.has_value() )
    {
        data[ "has_bookmark" ] = true;
        data[ "bookmark" ]     = escapeHTML( toString( element.m_bookmark.value() ) );
    }
}

template < typename T >
inline bool addOptionalLink( HTMLTemplateEngine&, T& element, nlohmann::json& data )
{
    if( element.m_url.has_value() )
    {
        data[ "has_link" ] = true;
        data[ "link" ]     = javascriptHREF( element.m_url.value() );
        return true;
    }
    return false;
}

template < typename Value >
inline void renderLine( HTMLTemplateEngine& engine, const Line< Value >& line, std::ostream& os )
{
    nlohmann::json data( { { "style", "multiline_default" },
                           { "elements", nlohmann::json::array() },
                           { "has_bookmark", false },
                           { "has_link", false },
                           { "colour", line.m_colour.str() },
                           { "background_colour", line.m_background_colour.str() },
                           { "bookmark", "" } } );
    addOptionalBookmark( engine, line, data );
    if( addOptionalLink( engine, line, data ) )
    {
        // Args noLinkerArgs{ args.inja, nullptr };
        valueToJSON( engine, line.m_element, data[ "elements" ] );
    }
    else
    {
        valueToJSON( engine, line.m_element, data[ "elements" ] );
    }
    engine.render( HTMLTemplateEngine::eMultiLine, data, os );
}

template < typename Value >
inline void renderMultiline( HTMLTemplateEngine& engine, const Multiline< Value >& multiline, std::ostream& os )
{
    nlohmann::json data( { { "style", "multiline_default" },
                           { "elements", nlohmann::json::array() },
                           { "has_bookmark", false },
                           { "has_link", false },
                           { "colour", multiline.m_colour.str() },
                           { "background_colour", multiline.m_background_colour.str() },
                           { "bookmark", "" } } );

    addOptionalBookmark( engine, multiline, data );
    if( addOptionalLink( engine, multiline, data ) )
    {
        // Args noLinkerArgs{ args.inja, nullptr };
        valueVectorToJSON( engine, multiline.m_elements, data[ "elements" ] );
    }
    else
    {
        valueVectorToJSON( engine, multiline.m_elements, data[ "elements" ] );
    }

    engine.render( HTMLTemplateEngine::eMultiLine, data, os );
}

template < typename Value >
inline void renderContainer( HTMLTemplateEngine& engine, const Container< Value >& container, std::ostream& os );

template < typename Value >
inline void renderBranch( HTMLTemplateEngine& engine, const Branch< Value >& branch, std::ostream& os )
{
    nlohmann::json data( { { "style", "branch_default" },
                           { "has_bookmark", false },
                           { "bookmark", "" },
                           { "label", nlohmann::json::array() },
                           { "elements", nlohmann::json::array() } } );

    addOptionalBookmark( engine, branch, data );
    valueVectorToJSON( engine, branch.m_label, data[ "label" ] );

    for( const auto& pChildElement : branch.m_elements )
    {
        std::ostringstream osChild;
        renderContainer( engine, pChildElement, osChild );
        data[ "elements" ].push_back( osChild.str() );
    }

    engine.render( HTMLTemplateEngine::eBranch, data, os );
}

template < typename Value >
inline void renderTable( HTMLTemplateEngine& engine, const Table< Value >& table, std::ostream& os )
{
    nlohmann::json data( { { "headings", nlohmann::json::array() }, { "rows", nlohmann::json::array() } } );

    if( !table.m_headings.empty() )
    {
        valueVectorToJSON( engine, table.m_headings, data[ "headings" ] );
    }
    for( const auto& pRow : table.m_rows )
    {
        nlohmann::json row( { { "values", nlohmann::json::array() } } );
        for( const auto& pContainer : pRow )
        {
            std::ostringstream osChild;
            renderContainer( engine, pContainer, osChild );
            row[ "values" ].push_back( osChild.str() );
        }
        data[ "rows" ].push_back( row );
    }

    engine.render( HTMLTemplateEngine::eTable, data, os );
}

template < typename Value >
inline void renderPlot( HTMLTemplateEngine& engine, const Plot< Value >& plot, std::ostream& os )
{
    using namespace std::string_literals;

    nlohmann::json data( { { "headings", nlohmann::json::array() }, { "points", nlohmann::json::array() } } );

    valueVectorToJSON( engine, plot.m_heading, data[ "headings" ] );

    for( const auto& point : plot.m_points )
    {
        nlohmann::json values( { { "values", nlohmann::json::array() } } );

        valueVectorToJSON( engine, point, values[ "values" ] );

        data[ "points" ].push_back( values );
    }

    engine.render( HTMLTemplateEngine::ePlot, data, os );
}

template < typename Value >
inline void renderGraph( HTMLTemplateEngine& engine, const Graph< Value >& graph, std::ostream& os )
{
    using namespace std::string_literals;

    nlohmann::json data( { { "rank_direction", graph.m_rankDirection.str() },
                           { "nodes", nlohmann::json::array() },
                           { "edges", nlohmann::json::array() },
                           { "subgraphs", nlohmann::json::array() } } );

    std::vector< std::string > nodeNames;
    for( const auto& node : graph.m_nodes )
    {
        std::ostringstream osNodeName;
        osNodeName << "node_" << nodeNames.size();
        nodeNames.push_back( osNodeName.str() );

        nlohmann::json nodeData( {

            { "name", osNodeName.str() },
            { "colour", node.m_colour.str() },
            { "bgcolour", node.m_background_colour.str() },
            { "border_width", node.m_border_width },
            { "rows", nlohmann::json::array() },
            { "has_url", false }

        } );

        if( node.m_url.has_value() )
        {
            nodeData[ "has_url" ] = true;
            nodeData[ "url" ]     = javascriptHREF( node.m_url.value() );
        }

        // addOptionalBookmark( engine, node, nodeData );

        bool bFirst = true;
        for( const ValueVector< Value >& row : node.m_rows )
        {
            nlohmann::json rowData( { { "values", nlohmann::json::array() } } );
            for( const Value& value : row )
            {
                // NOTE graph value generates <td id="bookmark">value</td> so can generate href in graphviz
                if( bFirst )
                {
                    bFirst = false;
                    graphValueToJSON( engine, value, node.m_bookmark, rowData[ "values" ] );
                }
                else
                {
                    graphValueToJSON( engine, value, rowData[ "values" ] );
                }
            }
            nodeData[ "rows" ].push_back( rowData );
        }

        data[ "nodes" ].push_back( nodeData );
    }

    std::vector< std::string > subgraphNames;
    for( const auto& subgraph : graph.m_subgraphs )
    {
        std::ostringstream osNodeName;
        // NOTE: MUST start with 'cluster'
        osNodeName << "cluster_" << subgraphNames.size();
        subgraphNames.push_back( osNodeName.str() );

        nlohmann::json subgraphData( {

            { "name", osNodeName.str() },
            { "colour", subgraph.m_colour.str() },
            { "rows", nlohmann::json::array() },
            { "nodes", nlohmann::json::array() },
            { "has_url", false },
            { "has_label", false }

        } );

        if( subgraph.m_url.has_value() )
        {
            subgraphData[ "has_url" ] = true;
            subgraphData[ "url" ]     = javascriptHREF( subgraph.m_url.value() );
        }

        VERIFY_RTE_MSG( !subgraph.m_bookmark.has_value(), "Subgraph bookmark deprecated" );
        // addOptionalBookmark( engine, subgraph, subgraphData );

        for( const ValueVector< Value >& row : subgraph.m_rows )
        {
            nlohmann::json rowData( { { "values", nlohmann::json::array() } } );
            for( const Value& value : row )
            {
                // NOTE graph value generates <td>value</td> so can generate href in graphviz
                graphValueToJSON( engine, value, rowData[ "values" ] );
                subgraphData[ "has_label" ] = true;
            }
            subgraphData[ "rows" ].push_back( rowData );
        }

        for( auto iNode : subgraph.m_nodes )
        {
            VERIFY_RTE_MSG( ( iNode ) >= 0 && ( iNode < nodeNames.size() ), "Invalid subgraph node id of: " << iNode );
            subgraphData[ "nodes" ].push_back( nodeNames[ iNode ] );
        }

        data[ "subgraphs" ].push_back( subgraphData );
    }

    for( const auto& edge : graph.m_edges )
    {
        VERIFY_RTE_MSG( ( edge.m_source ) >= 0 && ( edge.m_source < nodeNames.size() ),
                        "Invalid edge node id of: " << edge.m_source );
        VERIFY_RTE_MSG( ( edge.m_target ) >= 0 && ( edge.m_target < nodeNames.size() ),
                        "Invalid edge node id of: " << edge.m_target );

        nlohmann::json edgeData( {

            { "from", nodeNames[ edge.m_source ] },
            { "to", nodeNames[ edge.m_target ] },
            { "colour", edge.m_colour.str() },
            { "style", edge.m_style.str() },
            { "constraint", edge.m_bIgnoreInLayout ? "false" : "true" },
            { "line_width", edge.line_width },
            { "has_label", edge.m_label.has_value() },
            { "label", nlohmann::json::array() }

        } );

        if( edge.m_label.has_value() )
        {
            for( const auto& value : edge.m_label.value() )
            {
                graphValueToJSON( engine, value, edgeData[ "label" ] );
            }
        }

        data[ "edges" ].push_back( edgeData );
    }

    engine.render( HTMLTemplateEngine::eGraph, data, os );
}

template < typename Value >
inline void renderContainer( HTMLTemplateEngine& engine, const Container< Value >& container, std::ostream& os )
{
    using namespace report;

    struct Visitor
    {
        HTMLTemplateEngine& engine;

        std::ostream& os;

        void operator()( const Line< Value >& line ) const { renderLine( engine, line, os ); }
        void operator()( const Multiline< Value >& multiline ) const { renderMultiline( engine, multiline, os ); }
        void operator()( const Branch< Value >& branch ) const { renderBranch( engine, branch, os ); }
        void operator()( const Table< Value >& table ) const { renderTable( engine, table, os ); }
        void operator()( const Plot< Value >& plot ) const { renderPlot( engine, plot, os ); }
        void operator()( const Graph< Value >& graph ) const { renderGraph( engine, graph, os ); }

    } visitor{ engine, os };

    std::visit( visitor, container );
}

template < typename Value >
inline void renderReport( HTMLTemplateEngine& engine, const Container< Value >& container, std::ostream& os )
{
    std::ostringstream osContainer;
    renderContainer( engine, container, osContainer );
    nlohmann::json report( { { "body", osContainer.str() }, { "reports", nlohmann::json::array() } } );

    /*
        for( const auto& reporterID : shortcuts.get() )
        {
            std::string strChar;
            strChar.push_back( reporterID.key );
            nlohmann::json reporterIDData( { { "key_char", strChar },
                                             { "key_code", javascriptKeyCode( reporterID.key ) },
                                             { "name", reporterID.strAction } } );
            report[ "reports" ].push_back( reporterIDData );
        }*/

    engine.render( HTMLTemplateEngine::eReport, report, os );
}

} // namespace detail

template < typename Value >
inline void renderHTML( const Container< Value >& report, std::ostream& os, HTMLTemplateEngine& engine )
{
    detail::renderReport( engine, report, os );
}

template < typename Value, typename Linker >
inline void renderHTML( const Container< Value >& report, std::ostream& os, Linker&, HTMLTemplateEngine& engine )
{
    detail::renderReport( engine, report, os );
}

template < typename Value >
inline void renderHTML( const Container< Value >& report, std::ostream& os )
{
    HTMLTemplateEngine engine{ false };
    renderHTML( report, os, engine );
}

template < typename Value, typename Linker >
inline void renderHTML( const Container< Value >& report, std::ostream& os, Linker& linker )
{
    HTMLTemplateEngine engine{ false };
    renderHTML( report, os, linker, engine );
}

} // namespace report

#endif // GUARD_2024_March_08_renderer_html