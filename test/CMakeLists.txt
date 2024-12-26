
set_source_files_properties( ${GENERATED_INTERFACE_FILES} PROPERTIES GENERATED TRUE )

add_library( test_component 
    ${GENERATED_INTERFACE_FILES}
    component.cpp
)
target_link_libraries( test_component Boost::program_options )
target_link_libraries( test_component Boost::fiber )

add_dependencies( test_component run_meta_pipeline )


