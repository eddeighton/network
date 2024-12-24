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

cmake_minimum_required( VERSION 3.1 )

project( Megastructure VERSION 1.0 LANGUAGES C CXX )

set( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib )
set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib )
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin )

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(Boost_USE_STATIC_LIBS ON)

enable_testing()

find_package( GTest REQUIRED )
find_package( Threads )
find_package( Boost 1.86 
    COMPONENTS
        program_options
        fiber
        iostreams
        filesystem
        timer
        serialization
        system
    REQUIRED )
find_package( nlohmann_json )
find_package( inja )

include( clangeg.cmake )

set(SRC_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${SRC_ROOT_DIR})

add_subdirectory( common )
add_subdirectory( database )
add_subdirectory( pipeline )
add_subdirectory( meta )

# run system wide meta pipeline

set(SERVICE_INTERFACES ${CMAKE_CURRENT_SOURCE_DIR}/test.interface.hpp)

set(GENERATED_INTERFACE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/test.cxx)

add_custom_command(
    OUTPUT ${GENERATED_INTERFACE_FILES}

    COMMAND meta
        --src_dir ${CMAKE_SOURCE_DIR}
        --build_dir ${CMAKE_BINARY_DIR}
        --stash_dir ${CMAKE_CURRENT_BINARY_DIR}/stash
        --templates ${CMAKE_CURRENT_SOURCE_DIR}/templates
        --pipeline "$<TARGET_FILE:meta_pipeline>"
        --interfaces \"${SERVICE_INTERFACES}\"
 
    BYPRODUCTS # ${GENERATED_INTERFACE_FILES}
    DEPENDS ${SERVICE_INTERFACES}
    COMMENT "Running meta pipeline"
)

add_library( meta_service_interface
    ${GENERATED_INTERFACE_FILES}
)



add_subdirectory( service )
add_subdirectory( controller )

