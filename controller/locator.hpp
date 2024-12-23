
#pragma once

#include "common/assert_verify.hpp"

#include <boost/spirit/home/support/iterators/line_pos_iterator.hpp>
#include <boost/variant.hpp>

#include <string>
#include <vector>
#include <optional>
#include <ostream>
#include <sstream>
#include <variant>

namespace Controller
{
using UnderlyingIterType = std::string::const_iterator;
using IteratorType       = boost::spirit::line_pos_iterator< UnderlyingIterType >;

struct ParseResult
{
    bool         bSuccess{};
    IteratorType iterReached{};
};

struct Name : public std::string
{
    class Compare
    {
    public:
        inline bool operator()( const Name& left, const Name& right ) const
        {
            return std::lexicographical_compare( left.begin(), left.end(), right.begin(), right.end() );
        }
    };

    Name() = default;
    Name( const std::string& str )
        : std::string( str )
    {
    }
    Name( const char* pszStr )
        : std::string( pszStr )
    {
    }
};

ParseResult parse( const std::string& strInput, Name& code, std::ostream& errorStream );

struct Extension : public std::string
{
    class Compare
    {
    public:
        inline bool operator()( const Extension& left, const Name& right ) const
        {
            return std::lexicographical_compare( left.begin(), left.end(), right.begin(), right.end() );
        }
    };

    Extension() = default;
    Extension( const std::string& str )
        : std::string( str )
    {
    }
    Extension( const char* pszStr )
        : std::string( pszStr )
    {
    }
};

ParseResult parse( const std::string& strInput, Extension& extension, std::ostream& errorStream );

struct Path
{
    static constexpr auto DELIMITER       = '/';
    static constexpr auto DIGIT_SEPARATOR = ':';

    class Compare
    {
    public:
        inline bool operator()( const Path& left, const Path& right ) const
        {
            if( left.m_relative != right.m_relative )
                return left.m_relative < right.m_relative;
            if( left.m_names != right.m_names )
                return std::lexicographical_compare( left.m_names.begin(), left.m_names.end(), right.m_names.begin(),
                                                     right.m_names.end(), Name::Compare{} );
            else if( left.m_extensions != right.m_extensions )
                return std::lexicographical_compare( left.m_extensions.begin(), left.m_extensions.end(),
                                                     right.m_extensions.begin(), right.m_extensions.end(),
                                                     Extension::Compare{} );
            else if( left.m_line != right.m_line )
                return left.m_line < right.m_line;
            else if( left.m_column != right.m_column )
                return left.m_column < right.m_column;
            else
                return left.m_byte < right.m_byte;
        }
    };

    inline bool operator==( const Path& cmp ) const = default;

    inline void to_path( std::ostream& os, const std::string strFilter = std::string{} ) const
    {
        if( m_relative )
        {
            os << m_relative.value();
        }
        for( const auto& n : m_names )
        {
            if( strFilter.empty() || (n != strFilter) )
            {
                os << Path::DELIMITER << n;
            }
        }
        for( const auto& e : m_extensions )
        {
            os << e;
        }
    }

    inline void to_path_with_digits( std::ostream& os ) const
    {
        to_path( os );
        if( m_line )
        {
            os << Path::DIGIT_SEPARATOR << m_line.value();
        }
        //if( m_column )
        //{
        //    os << Path::DIGIT_SEPARATOR << m_column.value();
        //}
        if( m_byte )
        {
            os << Path::DIGIT_SEPARATOR << m_byte.value();
        }
    }

    inline std::string str() const
    {
        std::ostringstream os;
        to_path_with_digits( os );
        return os.str();
    }

    std::optional< Name >    m_relative;
    std::vector< Name >      m_names;
    std::vector< Extension > m_extensions;
    std::optional< int >     m_line;
    std::optional< int >     m_column;
    std::optional< int >     m_byte;
};

ParseResult parse( const std::string& strInput, Path& path, std::ostream& errorStream );

inline std::ostream& operator<<( std::ostream& os, const Path& path )
{
    path.to_path_with_digits( os );
    return os;
}

struct Line
{
    class Compare
    {
    public:
        inline bool operator()( const Line& left, const Line& right ) const
        {
            return std::lexicographical_compare(
                left.m_paths.begin(), left.m_paths.end(), right.m_paths.begin(), right.m_paths.end(), Path::Compare{} );
        }
    };

    std::vector< Path > m_paths;
};

ParseResult parse( const std::string& strInput, Line& line, std::ostream& errorStream );

template < typename T >
T parse( const std::string& strInput )
{
    std::ostringstream osError;
    T                  resultType;
    const ParseResult  result = parse( strInput, resultType, osError );
    VERIFY_RTE_MSG( result.bSuccess && result.iterReached.base() == strInput.end(),
                    "Failed to parse string: " << strInput << " : " << osError.str() );
    return resultType;
}
} // namespace Controller

