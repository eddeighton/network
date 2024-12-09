include( ${CMAKE_SOURCE_DIR}/database/database.cmake )

set(ADVANCED_DIR ${CMAKE_CURRENT_BINARY_DIR}/db_test_advanced)

MegaDatabase( 
    DBNAME      database_advanced_tests
    DBCOMPILER  database_compiler_driver
    LIB         ${CMAKE_SOURCE_DIR}/database/compiler/lib 
    TEMPLATE    ${CMAKE_SOURCE_DIR}/database/compiler/templates
    DATA        ${ADVANCED_DIR}/data/
    API         ${ADVANCED_DIR}
    SRC         ${ADVANCED_DIR}
    SCHEMAS     ${CMAKE_CURRENT_SOURCE_DIR}/schema.dbs
    STAGES      FirstStage SecondStage ThirdStage FourthStage
)

set( DB_TESTS advanced_tests.cpp )

add_executable( database_advanced_tests 
                ${DATABASE_COPIED_FILES} 
                ${DATABASE_GENERATED_FILES} 
                ${DATABASE_GENERATED_STAGE_FILES} 
                ${DB_TESTS} 
                main.cpp 
)

add_dependencies( database_advanced_tests database_compiler_driver )

# allow access to source folder for tests - NOTE order sensitive here
target_include_directories( database_advanced_tests PUBLIC ${ADVANCED_DIR} )
target_include_directories( database_advanced_tests PUBLIC ${DB_API_DIR} )

target_link_libraries( database_advanced_tests Boost::filesystem )
target_link_libraries( database_advanced_tests Boost::iostreams )
target_link_libraries( database_advanced_tests Boost::program_options )
target_link_libraries( database_advanced_tests Boost::serialization )
target_link_libraries( database_advanced_tests pantor::inja )
target_link_libraries( database_advanced_tests nlohmann_json::nlohmann_json )
target_link_libraries( database_advanced_tests GTest::gtest GTest::gtest_main )
target_link_libraries( database_advanced_tests Common::commonlib ) 

gtest_add_tests( TARGET database_advanced_tests SOURCES ${DB_TESTS}  )

