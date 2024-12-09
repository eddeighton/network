
cmake_minimum_required(VERSION 3.2)

#set( COMMON_SOURCE_DIR ${COMMON_ROOT_DIR}/src )
file( GLOB COMMON_INCLUDE_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/*.hpp )
file( GLOB COMMON_SOURCE_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp )
set( COMMON_SOURCES ${COMMON_SOURCE_HEADERS} ${COMMON_SOURCE_SOURCES} )

source_group( src FILES ${COMMON_SOURCES} )

add_library( commonlib ${CFG_LINK}
    ${COMMON_SOURCES}
    # ${COMMON_SOURCE_DIR}/common/clang_tsan_memcpy.c
)
add_library( Common::commonlib ALIAS commonlib )
set_target_properties( commonlib PROPERTIES FOLDER common )

target_link_libraries( commonlib Boost::filesystem )
target_link_libraries( commonlib Boost::iostreams )
target_link_libraries( commonlib Boost::timer )

add_subdirectory( test )

