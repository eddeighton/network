

add_subdirectory( protocol )


add_executable( daemon daemon.cpp )
target_include_directories( daemon PRIVATE ${SRC_ROOT_DIR})
target_link_libraries( daemon Boost::program_options )
target_link_libraries( daemon Boost::fiber )

add_executable( mega mega.cpp )
target_include_directories( mega PRIVATE ${SRC_ROOT_DIR})
target_link_libraries( mega Boost::program_options )
target_link_libraries( mega Boost::fiber )

