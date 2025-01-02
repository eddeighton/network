
set_source_files_properties( ${GENERATED_INTERFACE_FILES} PROPERTIES GENERATED TRUE )
add_library( service ${GENERATED_INTERFACE_FILES}
    asio.cpp
    logical_thread.cpp
)

target_link_libraries( service Boost::serialization )
add_dependencies( service run_meta_pipeline )

add_executable( daemon daemon.cpp )
target_link_libraries( daemon service )
target_link_libraries( daemon Boost::program_options )
target_link_libraries( daemon Boost::fiber )

add_executable( mega mega.cpp asio.cpp )
target_link_libraries( mega service )
target_link_libraries( mega Boost::program_options )
target_link_libraries( mega Boost::fiber )
target_link_libraries( mega pybind11::embed )

