

add_library( meta_pipeline SHARED 

    ${DATABASE_COPIED_FILES} 
    ${DATABASE_GENERATED_FILES} 
    ${DATABASE_GENERATED_STAGE_FILES} 

    meta.cpp
    task_interface_analysis.cpp
    task_interface_analysis_report.cpp
    task_service_gen.cpp
)

add_dependencies( meta_pipeline database_compiler_driver )

# allow access to source folder for tests - NOTE order sensitive here
target_include_directories( meta_pipeline PUBLIC ${META_DB_PATH} )

target_link_libraries( meta_pipeline Boost::filesystem )
target_link_libraries( meta_pipeline Boost::iostreams )
target_link_libraries( meta_pipeline Boost::program_options )
target_link_libraries( meta_pipeline Boost::serialization )
target_link_libraries( meta_pipeline Boost::url )
target_link_libraries( meta_pipeline pantor::inja )
target_link_libraries( meta_pipeline nlohmann_json::nlohmann_json )
target_link_libraries( meta_pipeline Common::commonlib )
target_link_libraries( meta_pipeline reportlib)

link_clang_tooling( meta_pipeline )

