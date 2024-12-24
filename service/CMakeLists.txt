

add_subdirectory( protocol )

# set_property( SOURCE ${CMAKE_SOURCE_DIR}/meta/test.cxx PROPERTY GENERATED)
# add_executable( daemon daemon.cpp ${CMAKE_SOURCE_DIR}/meta/test.cxx) # meta/test.cxx

add_executable( daemon daemon.cpp ${CMAKE_SOURCE_DIR}/test.cxx)
target_include_directories( daemon PRIVATE ${SRC_ROOT_DIR})
target_link_libraries( daemon Boost::program_options )
target_link_libraries( daemon Boost::fiber )

add_executable( mega mega.cpp )
target_include_directories( mega PRIVATE ${SRC_ROOT_DIR})
target_link_libraries( mega Boost::program_options )
target_link_libraries( mega Boost::fiber )

