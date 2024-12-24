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

#include <gtest/gtest.h>

#include "report/report.hpp"
#include "report/renderer_html.hpp"

#include "common/file.hpp"

#include <sstream>

//extern 
static boost::filesystem::path g_resultDir = "/src/report/test";
//extern 
static boost::filesystem::path g_templateDir = "/src/report/templates";

enum Foobar
{
    eOne,
    eTwo
};
std::ostream& operator<<( std::ostream& os, Foobar v )
{
    switch( v )
    {
        case eOne:
            return os << "One";
        case eTwo:
            return os << "Two";
        default:
            THROW_RTE( "Unknown value of Foobar" );
            return os;
    }
}

TEST( Report, Basic )
{
    using namespace std::string_literals;

    using V = std::variant< int, double, std::string, Foobar >;

    using namespace report;

    using Cont = Container< V >;
    using B    = Branch< V >;
    using L    = Line< V >;
    using M    = Multiline< V >;
    using T    = Table< V >;
    using P    = Plot< V >;
    using G    = Graph< V >;

    // clang-format off
    Cont c =
    B
    { 
        { "Report.Basic"s }, 
        { 
            L{ "T1"s }, 
            B
            { 
                { "Nested"s }, 
                { 
                    M{ {  "T2a"s, " "s, eOne } }, 
                    P{ 
                        { "Plot"s, eOne },
                        {
                            { 1,    5.24950377963947,       "testing1"s },
                            { 2,    0.2768319460690982,     "testing2"s },
                            { 3,    2.458300290671558,      "testing3"s },
                            { 4,    8.074619729546166,      "testing4"s },
                            { 5,    3.556050859987454,      "testing5"s },
                            { 6,    9.784662051855879,      "testing6"s },
                            { 7,    0.06326750348053745,    "testing7"s },
                            { 8,    3.0619009840375355,     "testing8"s },
                            { 9,    4.712482948657854,      "testing9"s },
                            { 10,   5.785610242115469,      "testing10"s },
                            { 11,   0.5479191370529124,     "testing11"s },
                            { 12,   2.420990056711698,      "testing12"s },
                            { 13,   8.98839454120511,       "testing13"s },
                            { 14,   5.567374627104407,      "testing14"s },
                            { 15,   4.073801044545135,      "testing15"s },
                            { 16,   5.848917306708272,      "testing16"s }
                        }
                    }, 
                    P{ 
                        { "Plot"s, eTwo },
                        {
                            { 1,    5.24950377963947,       "testing1" },
                            { 2,    0.2768319460690982,     "testing2" },
                            { 3,    2.458300290671558,      "testing3" },
                            { 4,    8.074619729546166,      "testing4" },
                            { 5,    3.556050859987454,      "testing5" },
                            { 6,    9.784662051855879,      "testing6" },
                            { 7,    0.06326750348053745,    "testing7" },
                            { 8,    3.0619009840375355,     "testing8" },
                            { 9,    4.712482948657854,      "testing9" },
                            { 10,   5.785610242115469,      "testing10" },
                            { 11,   0.5479191370529124,     "testing11" },
                            { 12,   2.420990056711698,      "testing12" },
                            { 13,   8.98839454120511,       "testing13" },
                            { 14,   5.567374627104407,      "testing14" },
                            { 15,   4.073801044545135,      "testing15" },
                            { 16,   5.848917306708272,      "testing16" }
                        }
                    }
                } 
            }, 
            G
            {
                // nodes
                { 
                    { { { "Node1"s, eOne }, { 1, 2 } } },
                    { { { "Node2"s, eTwo } } },
                    { { { "Node3"s, eOne } } }  
                },
                // edges
                {
                    { 0, 1 },
                    { 1, 2, Colour::red },
                    { 2, 0, Colour::blue, G::Edge::Style::dotted }
                },
                // subgraphs
                {
                    {
                        {},
                        { 1, 2 }
                    }
                }
            },
            L{ "T3"s },
            T{ { "H1"s, "H2"s, "H3"s },
            {
                { L{ 1 }, L{ 2 }, L{ eOne } },
                { L{ 1 }, L{ 2 }, L{ eTwo } },
                { L{ 1 }, L{ 2 }, L{ eOne } },
                { L{ 1 }, L{ 2 }, L{ eTwo } }
            }}
        } 
    };
    // clang-format on

    std::ostringstream os;

    if( g_templateDir.empty() )
    {
        renderHTML( c, os );
    }
    else
    {
        HTMLTemplateEngine templateEngine{ g_templateDir, false };
        renderHTML( c, os, templateEngine );
    }

    auto pFile = boost::filesystem::createNewFileStream( g_resultDir / "basic.html" );
    *pFile << os.str();
}
