
#pragma once

#include "requireSemicolon.hpp"
#include "terminal.hpp"

#include <boost/current_function.hpp>

#include <iostream>
#include <iomanip>
#include <cstdio>
#include <thread>
#include <cstring>

namespace mega::service
{
inline void printLogLine( const char* pszFile,
                          int         line,
                          int         threadID,
                          const char* pszMsg,
                          const char* pszFunction )
{
    const auto padding = std::max(
        40
            - ( strlen( pszFile )
                + std::snprintf( NULL, 0, "%d", line ) + 1 ),
        std::size_t{ 1U } );

    std::ostringstream osURL;
    osURL << common::URL_BEGIN << "file://" << pszFile << ":" << line
          << common::URL_MIDDLE << pszFile << ":" << line
          << common::URL_END;

    std::cout
        // Thread ID
        << common::COLOUR_CYAN_BEGIN << "THREAD: " << std::hex << "0x"
        << std::this_thread::get_id() << ' ' << threadID
        << std::dec

        // FILE URL
        << common::COLOUR_YELLOW_BEGIN << " FILE: " << osURL.str()
        << std::string( padding, ' ' )

        // Message
        << common::COLOUR_BLUE_BEGIN << " MSG: " << std::left
        << std::setw( 40 ) << std::setfill( ' ' ) << pszMsg;

    // Function
    if( pszFunction )
    {
        std::cout << " FUNCTION: " << std::right << std::setw( 30 )
                  << std::setfill( ' ' ) << pszFunction;
    }

    // END
    std::cout << common::COLOUR_END << std::endl;
}

int getThreadID();

} // namespace mega::service

#define ENABLE_LOGGING

#ifdef ENABLE_LOGGING

#define LOG( msg )                                                   \
    DO_STUFF_AND_REQUIRE_SEMI_COLON(                                 \
        std::ostringstream osMsg; osMsg << msg;                      \
        mega::service::printLogLine( __FILE__,                       \
                                     __LINE__,                       \
                                     ::mega::service::getThreadID(), \
                                     osMsg.str().c_str(),            \
                                     nullptr ); )

#define LOG_FUNCTION( msg )                                          \
    DO_STUFF_AND_REQUIRE_SEMI_COLON(                                 \
        std::ostringstream osMsg; osMsg << msg;                      \
        mega::service::printLogLine( __FILE__,                       \
                                     __LINE__,                       \
                                     ::mega::service::getThreadID(), \
                                     osMsg.str().c_str(),            \
                                     BOOST_CURRENT_FUNCTION ); )

#else
#define LOG( msg )
#define LOG_FUNCTION( msg )
#endif
