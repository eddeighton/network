



set( PIPELINE_HEADERS
    build_hash_code.hpp
    configuration.hpp
    pipeline_result.hpp
    pipeline.hpp
    stash.hpp
    task.hpp
    version.hpp
)

set( PIPELINE_SOURCE
    pipeline.cpp
)

add_library( pipeline
    ${PIPELINE_HEADERS} 
    ${PIPELINE_SOURCE}
)


target_link_libraries( pipeline Boost::filesystem )
target_link_libraries( pipeline Boost::serialization )
target_link_libraries( pipeline commonlib)

add_subdirectory( test )

