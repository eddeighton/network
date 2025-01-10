
#include "controller/locator.hpp"

#include "common/string.hpp"

#include <boost/algorithm/string/detail/formatter.hpp>

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/qi_eoi.hpp>
#include <boost/spirit/include/support_line_pos_iterator.hpp>
#include <boost/spirit/include/qi_lexeme.hpp>

#include <boost/phoenix/core.hpp>
#include <boost/phoenix/stl.hpp>
#include <boost/phoenix/fusion.hpp>
#include <boost/phoenix/operator.hpp>
#include <boost/phoenix/object.hpp>

#include <boost/fusion/include/adapt_struct.hpp>

#include <boost/variant/detail/apply_visitor_unary.hpp>

namespace Controller
{
template < typename Iterator >
struct error_handler
{
    template < typename, typename, typename, typename >
    struct result
    {
        typedef void type;
    };

    std::ostream& errorStream;
    error_handler( std::ostream& _errorStream )
        : errorStream( _errorStream )
    {
    }

    void operator()( Iterator, Iterator, Iterator err_pos,
                     boost::spirit::info const& what ) const
    {
        const int line = boost::spirit::get_line( err_pos );
        if( line != -1 )
            errorStream << '(' << line << ')';
        else
            errorStream << "( unknown )";
        errorStream << " : Error! Expected " << what << std::endl;
    }
};

template < template < typename IterType > class GrammarType,
           typename ResultType >
inline ParseResult parse_impl( const std::string& strInput,
                               ResultType&        result,
                               std::ostream&      errorStream )
{
    GrammarType< IteratorType > grammar;
    {
        using namespace boost::phoenix;
        function< error_handler< IteratorType > > const error_handler(
            errorStream );
        boost::spirit::qi::on_error< boost::spirit::qi::fail >(
            grammar.m_main_rule,
            error_handler(
                boost::spirit::qi::_1, boost::spirit::qi::_2,
                boost::spirit::qi::_3, boost::spirit::qi::_4 ) );
    }

    ParseResult resultPair{ false, IteratorType( strInput.begin() ) };

    resultPair.bSuccess = boost::spirit::qi::phrase_parse(
        resultPair.iterReached, IteratorType( strInput.end() ),
        grammar, boost::spirit::eol,
        boost::spirit::qi::skip_flag::dont_postskip, result );

    return resultPair;
}

static const char* valid_name_chars_ = "a-zA-Z0-9_-";

template < typename Iterator >
class NameGrammar
    : public boost::spirit::qi::grammar< Iterator, Name() >
{
public:
    NameGrammar()
        : NameGrammar::base_type( m_main_rule, "name" )
    {
        using namespace boost::spirit;
        using namespace boost::spirit::qi;
        using namespace boost::phoenix;
        m_main_rule = +char_(
            valid_name_chars_ )[ push_back( _val, qi::_1 ) ];
    }

    boost::spirit::qi::rule< Iterator, Name() > m_main_rule;
};

ParseResult parse( const std::string& strInput, Name& code,
                   std::ostream& errorStream )
{
    return parse_impl< NameGrammar >( strInput, code, errorStream );
}

template < typename Iterator >
class ExtensionGrammar
    : public boost::spirit::qi::grammar< Iterator, Extension() >
{
public:
    ExtensionGrammar()
        : ExtensionGrammar::base_type( m_main_rule, "extension" )
    {
        using namespace boost::spirit;
        using namespace boost::spirit::qi;
        using namespace boost::phoenix;
        m_main_rule
            = char_( '.' )[ push_back( _val, qi::_1 ) ] >> +( char_(
                  valid_name_chars_ )[ push_back( _val, qi::_1 ) ] );
    }

    boost::spirit::qi::rule< Iterator, Extension() > m_main_rule;
};

ParseResult parse( const std::string& strInput, Extension& extension,
                   std::ostream& errorStream )
{
    return parse_impl< ExtensionGrammar >(
        strInput, extension, errorStream );
}

} // namespace Controller

// clang-format off
BOOST_FUSION_ADAPT_STRUCT( Controller::Path,
   ( std::optional< Controller::Name >, m_relative)
   ( std::vector< Controller::Name >, m_names)
   ( std::vector< Controller::Extension >, m_extensions)
   ( std::optional< int >, m_line)
   ( std::optional< int >, m_column)
   ( std::optional< int >, m_byte)
)

// clang-format on

namespace Controller
{

template < typename Iterator >
class PathGrammar
    : public boost::spirit::qi::grammar< Iterator, Path() >
{
public:
    PathGrammar()
        : PathGrammar::base_type( m_main_rule, "path" )
    {
        using namespace boost::spirit;
        using namespace boost::spirit::qi;
        using namespace boost::phoenix;
        m_main_rule
            = -m_name_rule[ at_c< 0 >( _val ) = qi::_1 ]
              >> *( lit( Path::DELIMITER ) >> m_name_rule[ push_back(
                        at_c< 1 >( _val ), qi::_1 ) ] )
              >> +( m_extension_rule[ push_back(
                  at_c< 2 >( _val ), qi::_1 ) ] )
              >> -(
                  lit( Path::DIGIT_SEPARATOR )
                  >> int_[ at_c< 3 >( _val ) = qi::_1 ] >> -(
                      lit( Path::DIGIT_SEPARATOR )
                      >> int_[ at_c< 4 >( _val ) = qi::_1 ] >> -(
                          lit( Path::DIGIT_SEPARATOR )
                          >> int_[ at_c< 5 >( _val ) = qi::_1 ] ) ) );
    }

    NameGrammar< Iterator >                     m_name_rule;
    ExtensionGrammar< Iterator >                m_extension_rule;
    boost::spirit::qi::rule< Iterator, Path() > m_main_rule;
};

ParseResult parse( const std::string& strInput, Path& extension,
                   std::ostream& errorStream )
{
    return parse_impl< PathGrammar >(
        strInput, extension, errorStream );
}

} // namespace Controller

// clang-format off
BOOST_FUSION_ADAPT_STRUCT( Controller::Line,
    ( std::vector< Controller::Path >, m_paths ) )
// clang-format on

namespace Controller
{

template < typename Iterator >
class LineGrammar
    : public boost::spirit::qi::grammar< Iterator, Line() >
{
public:
    LineGrammar()
        : LineGrammar::base_type( m_main_rule, "path" )
    {
        using namespace boost::spirit;
        using namespace boost::spirit::qi;
        using namespace boost::phoenix;
        m_main_rule
            = *( m_path_rule[ push_back( at_c< 0 >( _val ), qi::_1 ) ]
                 | char_ );
    }

    PathGrammar< Iterator >                     m_path_rule;
    boost::spirit::qi::rule< Iterator, Line() > m_main_rule;
};

ParseResult parse( const std::string& strInput, Line& line,
                   std::ostream& errorStream )
{
    return parse_impl< LineGrammar >( strInput, line, errorStream );
}

} // namespace Controller
