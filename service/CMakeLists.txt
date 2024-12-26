

add_subdirectory( protocol )

add_executable( daemon daemon.cpp asio.cpp )
target_link_libraries( daemon Boost::program_options )
target_link_libraries( daemon Boost::fiber )

add_executable( mega mega.cpp asio.cpp )
target_link_libraries( mega Boost::program_options )
target_link_libraries( mega Boost::fiber )
target_link_libraries( mega test_component )


