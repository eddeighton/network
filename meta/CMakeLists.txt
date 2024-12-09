
include( ${CMAKE_SOURCE_DIR}/database/database.cmake )


MegaDatabase( 
    DBNAME      meta
    DBCOMPILER  database_compiler_driver
    LIB         ${CMAKE_SOURCE_DIR}/database/compiler/lib 
    TEMPLATE    ${CMAKE_SOURCE_DIR}/database/compiler/templates
    DATA        ${CMAKE_CURRENT_SOURCE_DIR}/db/data/
    API         ${CMAKE_CURRENT_SOURCE_DIR}/db
    SRC         ${CMAKE_CURRENT_SOURCE_DIR}/db
    SCHEMAS     ${CMAKE_CURRENT_SOURCE_DIR}/schema.dbs
    STAGES      AnalysisStage 
)



add_executable( meta 

                ${DATABASE_COPIED_FILES} 
                ${DATABASE_GENERATED_FILES} 
                ${DATABASE_GENERATED_STAGE_FILES} 

                main.cpp 
                )

add_dependencies( meta database_compiler_driver )

# allow access to source folder for tests - NOTE order sensitive here
target_include_directories( meta PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/db )

target_link_libraries( meta Boost::filesystem )
target_link_libraries( meta Boost::iostreams )
target_link_libraries( meta Boost::program_options )
target_link_libraries( meta Boost::serialization )
target_link_libraries( meta pantor::inja )
target_link_libraries( meta nlohmann_json::nlohmann_json )
target_link_libraries( meta GTest::gtest GTest::gtest_main )
target_link_libraries( meta Common::commonlib )

