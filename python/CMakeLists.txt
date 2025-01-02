

set( MEGA_PYTHON_SOURCE

    ${CMAKE_CURRENT_SOURCE_DIR}/module.cpp
)

add_library( megastructure SHARED ${MEGA_PYTHON_SOURCE} )

target_link_libraries( megastructure service )
target_link_libraries( megastructure Boost::fiber )
target_link_libraries( megastructure Boost::serialization )

target_include_directories( megastructure PRIVATE ${Python3_INCLUDE_DIRS} )

pybind11_extension( megastructure )

