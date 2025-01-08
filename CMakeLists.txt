cmake_minimum_required( VERSION 3.1 )

project( Megastructure VERSION 1.0 LANGUAGES C CXX )

set( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib )
set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib )
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin )

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(Boost_USE_STATIC_LIBS ON)

# always enable some gcc diagnostic options
add_compile_options(-fdiagnostics-color)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# TODO: fix warnings
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
find_package( Python3 REQUIRED COMPONENTS Interpreter Development)
find_package( pybind11 )

include( clangeg.cmake )

set(SRC_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${SRC_ROOT_DIR})

add_subdirectory( common )
add_subdirectory( vocab )
add_subdirectory( database )
add_subdirectory( pipeline )
add_subdirectory( meta )
add_subdirectory( report )

################################################################################
################################################################################
# run system wide meta pipeline
set(SERVICE_INTERFACES 
    ${CMAKE_SOURCE_DIR}/service/interface/test.interface.hpp
    ${CMAKE_SOURCE_DIR}/service/interface/connectivity.interface.hpp
)

set(GENERATED_INTERFACE_FILES 
    ${CMAKE_SOURCE_DIR}/service/gen/proxy_itc.hxx
    ${CMAKE_SOURCE_DIR}/service/gen/proxy_ipc.hxx
    ${CMAKE_SOURCE_DIR}/service/gen/decoder.hxx
    ${CMAKE_SOURCE_DIR}/service/gen/registry.hxx
)

set(GENERATED_PYTHON_FILES
    ${CMAKE_SOURCE_DIR}/python/module.cxx
)

set(META_PIPELINE_TEMPLATES
    ${CMAKE_SOURCE_DIR}/meta/pipeline/templates/decoder.hxx.jinja
    ${CMAKE_SOURCE_DIR}/meta/pipeline/templates/proxy_itc.hxx.jinja
    ${CMAKE_SOURCE_DIR}/meta/pipeline/templates/proxy_ipc.hxx.jinja
    ${CMAKE_SOURCE_DIR}/meta/pipeline/templates/registry.hxx.jinja
    ${CMAKE_SOURCE_DIR}/meta/pipeline/templates/module.cxx.jinja
)

message( STATUS "Generated interface files: ${GENERATED_INTERFACE_FILES}" )
message( STATUS "Generated python files: ${GENERATED_PYTHON_FILES}" )

add_custom_command(
    OUTPUT ${GENERATED_INTERFACE_FILES} ${GENERATED_PYTHON_FILES}

    COMMAND meta
        --src_dir ${CMAKE_SOURCE_DIR}
        --build_dir ${CMAKE_BINARY_DIR}
        --stash_dir ${CMAKE_CURRENT_BINARY_DIR}/stash
        --templates ${CMAKE_SOURCE_DIR}/meta/pipeline/templates
        --pipeline "$<TARGET_FILE:meta_pipeline>"
        --interfaces \"${SERVICE_INTERFACES}\"
 
    DEPENDS ${SERVICE_INTERFACES} ${META_PIPELINE_TEMPLATES} meta_pipeline driver
    COMMENT "Running meta pipeline"
)

add_custom_target( run_meta_pipeline
    DEPENDS ${GENERATED_INTERFACE_FILES}
)

################################################################################
################################################################################
# The service
add_subdirectory( service )

################################################################################
################################################################################
# Python support
add_subdirectory( python )

################################################################################
################################################################################
# Service Components
add_subdirectory( controller )
add_subdirectory( test )

