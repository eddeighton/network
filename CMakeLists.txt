cmake_minimum_required( VERSION 3.1 )

project( Megastructure VERSION 1.0 LANGUAGES C CXX )

set( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib )
set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib )
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin )

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(Boost_USE_STATIC_LIBS ON)

# always enable some gcc diagnostic options
add_compile_options(-fdiagnostics-color)
add_compile_options(-fdiagnostics-urls=always)

# strict warnings cos why not!
# add_compile_options(-Wall -Wextra -Wpedantic )

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
        url
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
add_subdirectory( report )

################################################################################
################################################################################
# run system wide meta pipeline
set(SERVICE_INTERFACES ${CMAKE_SOURCE_DIR}/test/test.interface.hpp)
set(GENERATED_INTERFACE_FILES ${CMAKE_SOURCE_DIR}/test/service/test.cxx)

message( STATUS "Generated interface files: ${GENERATED_INTERFACE_FILES}" )

add_custom_command(
    OUTPUT ${GENERATED_INTERFACE_FILES}

    COMMAND meta
        --src_dir ${CMAKE_SOURCE_DIR}
        --build_dir ${CMAKE_BINARY_DIR}
        --stash_dir ${CMAKE_CURRENT_BINARY_DIR}/stash
        --templates ${CMAKE_CURRENT_SOURCE_DIR}/templates
        --pipeline "$<TARGET_FILE:meta_pipeline>"
        --interfaces \"${SERVICE_INTERFACES}\"
 
    DEPENDS ${SERVICE_INTERFACES} meta_pipeline
    COMMENT "Running meta pipeline"
)
set(SERVICE_CODE_GEN ${CMAKE_SOURCE_DIR}/test.cxx)

add_custom_target( run_meta_pipeline
    DEPENDS ${GENERATED_INTERFACE_FILES}
)

################################################################################
################################################################################
# The service
add_subdirectory( service )

################################################################################
################################################################################
# Service Components
add_subdirectory( controller )
add_subdirectory( test )

