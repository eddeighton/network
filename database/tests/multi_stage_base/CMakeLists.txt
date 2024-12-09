



include( ${CMAKE_SOURCE_DIR}/database/database.cmake )

set(DB_TEST_DIR ${CMAKE_CURRENT_BINARY_DIR}/db_test_multi_stage_base)

MegaDatabase( 
    DBNAME      database_multi_stage_base_tests
    DBCOMPILER  database_compiler_driver
    LIB         ${CMAKE_SOURCE_DIR}/database/compiler/lib 
    TEMPLATE    ${CMAKE_SOURCE_DIR}/database/compiler/templates
    DATA        ${DB_TEST_DIR}/data/
    API         ${DB_TEST_DIR}
    SRC         ${DB_TEST_DIR}
    SCHEMAS     ${CMAKE_CURRENT_SOURCE_DIR}/schema.dbs
    STAGES      FirstStage SecondStage ThirdStage
)

set( DB_BASIC_UNIT_TESTS
	multi_stage_base_tests.cpp 
    )

add_executable( database_multi_stage_base_tests 

                ${DATABASE_COPIED_FILES} 
                ${DATABASE_GENERATED_FILES} 
                ${DATABASE_GENERATED_STAGE_FILES} 

                ${DB_BASIC_UNIT_TESTS} 
                main.cpp 
                )

add_dependencies( database_multi_stage_base_tests database_compiler_driver )

# allow access to source folder for tests - NOTE order sensitive here
target_include_directories( database_multi_stage_base_tests PUBLIC ${DB_TEST_DIR} )
target_include_directories( database_multi_stage_base_tests PUBLIC ${DB_API_DIR} )

target_link_libraries( database_multi_stage_base_tests Boost::filesystem )
target_link_libraries( database_multi_stage_base_tests Boost::iostreams )
target_link_libraries( database_multi_stage_base_tests Boost::program_options )
target_link_libraries( database_multi_stage_base_tests Boost::serialization )
target_link_libraries( database_multi_stage_base_tests pantor::inja )
target_link_libraries( database_multi_stage_base_tests nlohmann_json::nlohmann_json )
target_link_libraries( database_multi_stage_base_tests GTest::gtest GTest::gtest_main )
target_link_libraries( database_multi_stage_base_tests Common::commonlib ) 

gtest_add_tests( TARGET database_multi_stage_base_tests SOURCES ${DB_BASIC_UNIT_TESTS}  )

