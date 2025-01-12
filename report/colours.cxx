
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

#include "report/colours.hxx"

#include <array>
#include <string>

namespace report
{
namespace
{
    static std::array< const char*, Colour::TOTAL_COLOURS > g_colourNames =
    {
        "aliceblue",
        "antiquewhite",
        "aqua",
        "aquamarine",
        "azur",
        "beige",
        "bisque",
        "black",
        "blanchedalmond",
        "blue",
        "blueviolet",
        "brown",
        "burlywood",
        "cadetblue",
        "chartreus",
        "chocolate",
        "coral",
        "cornflowerblue",
        "cornsilk",
        "crimso",
        "cyan",
        "darkblue",
        "darkcyan",
        "darkgoldenrod",
        "darkgra",
        "darkgreen",
        "darkgrey",
        "darkkhaki",
        "darkmagenta",
        "darkolivegree",
        "darkorange",
        "darkorchid",
        "darkred",
        "darksalmon",
        "darkseagree",
        "darkslateblue",
        "darkslategray",
        "darkslategrey",
        "darkturquoise",
        "darkviole",
        "deeppink",
        "deepskyblue",
        "dimgray",
        "dimgrey",
        "dodgerblu",
        "firebrick",
        "floralwhite",
        "forestgreen",
        "fuchsia",
        "gainsbor",
        "ghostwhite",
        "gold",
        "goldenrod",
        "gray",
        "gre",
        "green",
        "greenyellow",
        "honeydew",
        "hotpink",
        "indianre",
        "indigo",
        "ivory",
        "khaki",
        "lavender",
        "lavenderblus",
        "lawngreen",
        "lemonchiffon",
        "lightblue",
        "lightcoral",
        "lightcya",
        "lightgoldenrodyellow",
        "lightgray",
        "lightgreen",
        "lightgrey",
        "lightpin",
        "lightsalmon",
        "lightseagreen",
        "lightskyblue",
        "lightslategray",
        "lightslategre",
        "lightsteelblue",
        "lightyellow",
        "lime",
        "limegreen",
        "line",
        "magenta",
        "maroon",
        "mediumaquamarine",
        "mediumblue",
        "mediumorchi",
        "mediumpurple",
        "mediumseagreen",
        "mediumslateblue",
        "mediumspringgreen",
        "mediumturquois",
        "mediumvioletred",
        "midnightblue",
        "mintcream",
        "mistyrose",
        "moccasi",
        "navajowhite",
        "navy",
        "oldlace",
        "olive",
        "olivedra",
        "orange",
        "orangered",
        "orchid",
        "palegoldenrod",
        "palegree",
        "paleturquoise",
        "palevioletred",
        "papayawhip",
        "peachpuff",
        "per",
        "pink",
        "plum",
        "powderblue",
        "purple",
        "red",
        "rosybrown",
        "royalblue",
        "saddlebrown",
        "salmon",
        "sandybrow",
        "seagreen",
        "seashell",
        "sienna",
        "silver",
        "skyblu",
        "slateblue",
        "slategray",
        "slategrey",
        "snow",
        "springgree",
        "steelblue",
        "tan",
        "teal",
        "thistle",
        "tomat",
        "turquoise",
        "violet",
        "wheat",
        "white",
        "whitesmok",
        "yellow",
        "yellowgree",
    };
}

const char* Colour::str() const
{
    return g_colourNames[ m_type ];
}

}
