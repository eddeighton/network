

#ifndef LOG_30_11_2016
#define LOG_30_11_2016

#include "requireSemicolon.hpp"
#include "terminal.hpp"

#include <boost/current_function.hpp>

#include <iostream>

#define LOG( msg ) \
    DO_STUFF_AND_REQUIRE_SEMI_COLON( \
            std::cout << common::COLOUR_YELLOW_BEGIN << \
            "FILE: " << __FILE__ << ":" << __LINE__ << \
            " FUNCTION: " << BOOST_CURRENT_FUNCTION << \
            " MSG: " << msg << \
            common::COLOUR_END << "\n"; \
        )

#endif //LOG_30_11_2016


