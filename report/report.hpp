
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

#ifndef GUARD_2023_October_17_reports
#define GUARD_2023_October_17_reports

#include "report/value.hpp"
#include "report/url.hpp"
#include "report/colours.hxx"

#include "common/serialisation.hpp"

#include <vector>
#include <ostream>
#include <optional>
#include <variant>

namespace report
{

template < typename Value >
class Line;

template < typename Value >
class Multiline;

template < typename Value >
class Branch;

template < typename Value >
class Table;

template < typename Value >
class Plot;

template < typename Value >
class Graph;

template < typename Value >
using Container
    = std::variant< Line< Value >, Multiline< Value >, Branch< Value >, Table< Value >, Plot< Value >, Graph< Value > >;

template < typename Value >
using ContainerVector = std::vector< Container< Value > >;

/***
    Line{ Value, opt< URL >, opt< Bookmark > }
*/
template < typename Value >
class Line
{
    friend class boost::serialization::access;

    template < class Archive >
    inline void serialize( Archive& archive, const unsigned int )
    {
        archive& m_element;
        archive& m_url;
        archive& m_bookmark;
    }

public:
    using ValueType = Value;

    Value                  m_element;
    std::optional< URL >   m_url;
    std::optional< Value > m_bookmark;
    Colour                 m_colour            = Colour::black;
    Colour                 m_background_colour = Colour::white;
};

// Line deduction guide
template < typename Value >
Line( Value ) -> Line< Value >;

template < typename Value >
Line( Value, std::optional< URL > ) -> Line< Value >;

template < typename Value >
Line( Value, std::optional< URL >, std::optional< Value > ) -> Line< Value >;

template < typename Value >
Line( Value, std::optional< URL >, std::optional< Value >, Colour ) -> Line< Value >;

template < typename Value >
Line( Value, std::optional< URL >, std::optional< Value >, Colour, Colour ) -> Line< Value >;

/***
    Multiline{ vec< Value >, opt< URL >, opt< Bookmark > }
*/
template < typename Value >
class Multiline
{
    friend class boost::serialization::access;
    template < class Archive >
    inline void serialize( Archive& archive, const unsigned int )
    {
        archive& m_elements;
        archive& m_url;
        archive& m_bookmark;
    }

public:
    using ValueType = Value;

    ValueVector< Value >   m_elements;
    std::optional< URL >   m_url;
    std::optional< Value > m_bookmark;
    Colour                 m_colour            = Colour::black;
    Colour                 m_background_colour = Colour::white;
};

// Multiline deduction guide
template < typename Value >
Multiline( ValueVector< Value > ) -> Multiline< Value >;

template < typename Value >
Multiline( ValueVector< Value >, std::optional< URL > ) -> Multiline< Value >;

template < typename Value >
Multiline( ValueVector< Value >, std::optional< URL >, std::optional< Value > ) -> Multiline< Value >;

template < typename Value >
Multiline( ValueVector< Value >, std::optional< URL >, std::optional< Value >, Colour ) -> Multiline< Value >;

template < typename Value >
Multiline( ValueVector< Value >, std::optional< URL >, std::optional< Value >, Colour, Colour ) -> Multiline< Value >;

/***
    Branch{ vec< Value >, vec< Container >, opt< Bookmark > }
*/
template < typename Value >
class Branch
{
    friend class boost::serialization::access;
    template < class Archive >
    inline void serialize( Archive& archive, const unsigned int )
    {
        archive& m_label;
        archive& m_elements;
        archive& m_bookmark;
    }

public:
    using ValueType = Value;

    ValueVector< Value >     m_label;
    ContainerVector< Value > m_elements;
    std::optional< Value >   m_bookmark;
};

// Branch deduction guide
template < typename Value >
Branch( ValueVector< Value > label ) -> Branch< Value >;

template < typename Value >
Branch( ValueVector< Value >, ContainerVector< Value > ) -> Branch< Value >;

template < typename Value >
Branch( ValueVector< Value >, ContainerVector< Value >, std::optional< Value > ) -> Branch< Value >;

/***
    Table{ vec< Value >, vec< vec< Container > > }

    NOTE: If headings can be left empty and heading row will be omitted
*/
template < typename Value >
class Table
{
    friend class boost::serialization::access;
    template < class Archive >
    inline void serialize( Archive& archive, const unsigned int )
    {
        archive& m_headings;
        archive& m_rows;
    }

public:
    using ValueType = Value;

    ValueVector< Value >                    m_headings;
    std::vector< ContainerVector< Value > > m_rows;
};

// Table deduction guide
template < typename Value >
Table( ValueVector< Value > label ) -> Table< Value >;

template < typename Value >
Table( ValueVector< Value >, std::vector< ContainerVector< Value > > ) -> Table< Value >;

/***
    Plot{ vec< Value >, vec< Point > }
*/
template < typename Value >
class Plot
{
    friend class boost::serialization::access;
    template < class Archive >
    inline void serialize( Archive& archive, const unsigned int )
    {
        archive& m_heading;
        archive& m_points;
    }

public:
    using ValueType = Value;
    using Point     = ValueVector< Value >;

    ValueVector< Value > m_heading;
    std::vector< Point > m_points;
};

/***
    Graph{ vec< Node >, vec< Edge > }

    Node{ vec< vec< Value > >, colour( lightblue ), opt< URL >, opt< Bookmark > }
    Edge{ sourceIndex, targetIndex, style }

    Edge Styles: dashed, dotted, solid, invis, bold
*/
template < typename Value >
class Graph
{
    friend class boost::serialization::access;
    template < class Archive >
    inline void serialize( Archive& archive, const unsigned int )
    {
        archive& m_nodes;
        archive& m_edges;
        archive& m_subgraphs;
        archive& m_rankDirection;
    }

public:
    using ValueType = Value;

    class RankDirection
    {
    public:
        enum Type
        {
            LR,
            RL,
            TB,
            BT,
            TOTAL_TYPES
        };

        RankDirection() = default;

        inline constexpr RankDirection( Type type )
            : m_value( type )
        {
        }

        inline const char* str() const
        {
            static const std::array< const char*, TOTAL_TYPES > g_types = { "LR", "RL", "TB", "BT" };
            return g_types[ m_value ];
        }

        template < class Archive >
        inline void serialize( Archive& archive, const unsigned int )
        {
            archive& m_value;
        }

    private:
        Type m_value = LR;
    };

    class Node
    {
        friend class boost::serialization::access;
        template < class Archive >
        inline void serialize( Archive& archive, const unsigned int )
        {
            archive& m_rows;
            archive& m_colour;
            archive& m_url;
            archive& m_bookmark;
            archive& m_background_colour;
            archive& m_border_width;
        }

    public:
        using ID     = std::size_t;
        using Vector = std::vector< Node >;

        std::vector< ValueVector< Value > > m_rows;

        Colour                 m_colour = Colour::blue;
        std::optional< URL >   m_url;
        std::optional< Value > m_bookmark;
        Colour                 m_background_colour = Colour::lightblue;
        int                    m_border_width      = 1;
    };

    class Subgraph
    {
        friend class boost::serialization::access;
        template < class Archive >
        inline void serialize( Archive& archive, const unsigned int )
        {
            archive& m_rows;
            archive& m_colour;
            archive& m_url;
            archive& m_bookmark;
            archive& m_nodes;
        }

    public:
        using Vector = std::vector< Subgraph >;

        std::vector< ValueVector< Value > > m_rows;
        std::vector< typename Node::ID >    m_nodes;

        Colour                 m_colour = Colour::lightblue;
        std::optional< URL >   m_url;
        std::optional< Value > m_bookmark;
    };

    class Edge
    {
        friend class boost::serialization::access;
        template < class Archive >
        inline void serialize( Archive& archive, const unsigned int )
        {
            archive& m_source;
            archive& m_target;
            archive& m_colour;
            archive& m_style;
            archive& m_bIgnoreInLayout;
            archive& line_width;
        }

    public:
        using Vector = std::vector< Edge >;

        class Style
        {
        public:
            enum Type
            {
                dashed,
                dotted,
                solid,
                invis,
                bold,
                TOTAL_STYLES
            };

            Style() = default;

            inline constexpr Style( Type type )
                : m_style( type )
            {
            }

            inline const char* str() const
            {
                static const std::array< const char*, TOTAL_STYLES > g_styles
                    = { "dashed", "dotted", "solid", "invis", "bold" };
                return g_styles[ m_style ];
            }

            template < class Archive >
            inline void serialize( Archive& archive, const unsigned int )
            {
                archive& m_style;
            }

        private:
            Type m_style = solid;
        };

        typename Node::ID m_source, m_target;

        Colour m_colour          = Colour::black;
        Style  m_style           = Style::solid;
        bool   m_bIgnoreInLayout = false;
        int    line_width        = 1;

        std::optional< std::vector< Value > > m_label;
    };

    typename Node::Vector     m_nodes;
    typename Edge::Vector     m_edges;
    typename Subgraph::Vector m_subgraphs;
    RankDirection             m_rankDirection;
};

} // namespace report

#endif // GUARD_2023_October_17_reports
