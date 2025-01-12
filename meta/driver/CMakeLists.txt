
set_source_files_properties( ${DATABASE_COPIED_FILES} PROPERTIES GENERATED TRUE )
set_source_files_properties( ${DATABASE_GENERATED_FILES} PROPERTIES GENERATED TRUE )
set_source_files_properties( ${DATABASE_GENERATED_STAGE_FILES} PROPERTIES GENERATED TRUE )

add_executable( meta 

    ${DATABASE_COPIED_FILES} 
    ${DATABASE_GENERATED_FILES} 
    ${DATABASE_GENERATED_STAGE_FILES} 

    main.cpp 
)


add_dependencies( meta database_compiler_driver )

# allow access to source folder for tests - NOTE order sensitive here
target_include_directories( meta PUBLIC ${META_DB_PATH} )

target_link_libraries( meta Boost::filesystem )
target_link_libraries( meta Boost::iostreams )
target_link_libraries( meta Boost::program_options )
target_link_libraries( meta Boost::serialization )
target_link_libraries( meta Boost::system )

target_link_libraries( meta pantor::inja )
target_link_libraries( meta nlohmann_json::nlohmann_json )
target_link_libraries( meta Common::commonlib )
target_link_libraries( meta pipeline )

