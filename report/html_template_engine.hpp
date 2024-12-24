
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

#ifndef GUARD_2024_March_11_html_template_engine
#define GUARD_2024_March_11_html_template_engine

#include <nlohmann/json.hpp>

#include <boost/filesystem/path.hpp>

#include <memory>
#include <array>
#include <string>
#include <map>

namespace inja
{
class Environment;
class Template;
} // namespace inja

/*
namespace nlohmann
{
template < typename T = void, typename SFINAE = void >
struct adl_serializer;

template < template < typename U, typename V, typename... Args > class ObjectType = std::map,
           template < typename U, typename... Args > class ArrayType = std::vector, class StringType = std::string,
           class BooleanType = bool, class NumberIntegerType = std::int64_t, class NumberUnsignedType = std::uint64_t,
           class NumberFloatType = double, template < typename U > class AllocatorType = std::allocator,
           template < typename T, typename SFINAE = void > class JSONSerializer = adl_serializer,
           class BinaryType                                                     = std::vector< std::uint8_t > >
class basic_json;

using json = basic_json<>;
} // namespace nlohmann
*/

namespace report
{

class HTMLTemplateEngine
{
    using EnvironmentPtr = std::unique_ptr< inja::Environment >;
    using TemplatePtr    = std::unique_ptr< inja::Template >;

    EnvironmentPtr m_pEnvironment;

public:
    enum TemplateType
    {
        eReport,
        eMultiLine,
        eBranch,
        eTable,
        ePlot,
        eGraph,
        TOTAL_TEMPLATE_TYPES
    };

private:
    std::array< std::string, TOTAL_TEMPLATE_TYPES > m_templateNames;
    boost::filesystem::path                         m_tempFolder;
    bool                                            m_bClearTempFiles;
    std::array< TemplatePtr, TOTAL_TEMPLATE_TYPES > m_templates;

    void renderTemplate( const nlohmann::json& data, TemplateType templateType, std::ostream& os );
    void renderPlot( const nlohmann::json& data, std::ostream& os );
    void renderGraph( const nlohmann::json& data, std::ostream& os );

public:
    HTMLTemplateEngine( bool bClearTempFiles );
    HTMLTemplateEngine( const boost::filesystem::path& templateDir, bool bClearTempFiles );
    ~HTMLTemplateEngine();

    void render( TemplateType templateType, const nlohmann::json& data, std::ostream& os );
};
} // namespace report

#endif // GUARD_2024_March_11_html_template_engine
