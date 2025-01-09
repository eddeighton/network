
#pragma once

#include "requireSemicolon.hpp"
#include "terminal.hpp"

#include <boost/current_function.hpp>

#include <iostream>
#include <iomanip>
#include <utility>
#include <cstdio>

#define LOG( msg )                                                 \
    DO_STUFF_AND_REQUIRE_SEMI_COLON(                               \
        const auto padding = std::max(                             \
            40                                                     \
                - ( strlen( __FILE__ )                             \
                    + std::snprintf( NULL, 0, "%d", __LINE__ )     \
                    + 1 ),                                         \
            std::size_t{ 1U } );                                   \
        std::ostringstream osURL;                                  \
        osURL << common::URL_BEGIN << "file://" << __FILE__ << ":" \
              << __LINE__ << common::URL_MIDDLE << __FILE__ << ":" \
              << __LINE__ << common::URL_END;                      \
        std::cout << common::COLOUR_YELLOW_BEGIN << "FILE: "       \
                  << osURL.str() << std::string( padding, ' ' )    \
                  << common::COLOUR_BLUE_BEGIN                     \
                  << " MSG: " << std::left << std::setw( 40 )      \
                  << std::setfill( ' ' ) << msg                    \
                  << common::COLOUR_END << std::endl; )

#define LOG_FUNCTION( msg )                                         \
    DO_STUFF_AND_REQUIRE_SEMI_COLON(                                \
        const auto padding = std::max(                              \
            40                                                      \
                - ( strlen( __FILE__ )                              \
                    + std::snprintf( NULL, 0, "%d", __LINE__ )      \
                    + 1 ),                                          \
            std::size_t{ 1U } );                                    \
        std::ostringstream osURL;                                   \
        osURL << common::URL_BEGIN << "file://" << __FILE__ << ":"  \
              << __LINE__ << common::URL_MIDDLE << __FILE__ << ":"  \
              << __LINE__ << common::URL_END;                       \
        std::cout << common::COLOUR_YELLOW_BEGIN << "FILE: "        \
                  << osURL.str() << std::string( padding, ' ' )     \
                  << common::COLOUR_BLUE_BEGIN                      \
                  << " MSG: " << std::left << std::setw( 40 )       \
                  << std::setfill( ' ' ) << msg                     \
                  << common::COLOUR_CYAN_BEGIN                      \
                  << " FUNCTION: " << std::right << std::setw( 30 ) \
                  << std::setfill( ' ' ) << BOOST_CURRENT_FUNCTION  \
                  << common::COLOUR_END << std::endl; )
