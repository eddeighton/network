##  Copyright (c) Deighton Systems Limited. 2025. All Rights Reserved.
##  Author: Edward Deighton
##  License: Please see license.txt in the project root folder.

##  Use and copying of this software and preparation of derivative works
##  based upon this software are permitted. Any copy of this software or
##  of any derivative work must include the above copyright notice, this
##  paragraph and the one after it.  Any distribution of this software or
##  derivative works must comply with all applicable laws.

##  This software is made available AS IS, and COPYRIGHT OWNERS DISCLAIMS
##  ALL WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE
##  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
##  PURPOSE, AND NOTWITHSTANDING ANY OTHER PROVISION CONTAINED HEREIN, ANY
##  LIABILITY FOR DAMAGES RESULTING FROM THE SOFTWARE OR ITS USE IS
##  EXPRESSLY DISCLAIMED, WHETHER ARISING IN CONTRACT, TORT (INCLUDING
##  NEGLIGENCE) OR STRICT LIABILITY, EVEN IF COPYRIGHT OWNERS ARE ADVISED
##  OF THE POSSIBILITY OF SUCH DAMAGES.


add_library( libcontroller
    locator.hpp
    locator.cpp
)

target_link_libraries( libcontroller Common::commonlib )

############################################################
############################################################
add_executable( controller
    main.cpp
)

target_link_libraries( controller libcontroller )

target_link_libraries( controller Boost::filesystem )
target_link_libraries( controller Boost::program_options )

############################################################
############################################################
add_executable( evdevtest test_evdev.cpp )

target_link_libraries( evdevtest libcontroller )

target_link_libraries( evdevtest Boost::filesystem )
target_link_libraries( evdevtest Boost::program_options )
target_link_libraries( evdevtest evdev )
#target_link_libraries( evdevtest /usr/lib/x86_64-linux-gnu/libevdev.a )
target_include_directories( evdevtest PUBLIC /usr/include/libevdev-1.0 )

############################################################
############################################################

add_subdirectory( test )

