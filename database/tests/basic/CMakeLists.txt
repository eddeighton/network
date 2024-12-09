



include( ${CMAKE_SOURCE_DIR}/database/database.cmake )

set(BASIC_DIR ${CMAKE_CURRENT_BINARY_DIR}/db_test_basic)

MegaDatabase( 
    DBNAME      database_basic_tests
    DBCOMPILER  database_compiler_driver
    LIB         ${CMAKE_SOURCE_DIR}/database/compiler/lib 
    TEMPLATE    ${CMAKE_SOURCE_DIR}/database/compiler/templates
    DATA        ${BASIC_DIR}/data/
    API         ${BASIC_DIR}
    SRC         ${BASIC_DIR}
    SCHEMAS     ${CMAKE_CURRENT_SOURCE_DIR}/schema.dbs
    STAGES      BasicStage SecondStage ThirdStage FourthStage
)

set( DB_BASIC_UNIT_TESTS
	basic_tests.cpp 
    )

add_executable( database_basic_tests 

                ${DATABASE_COPIED_FILES} 
                ${DATABASE_GENERATED_FILES} 
                ${DATABASE_GENERATED_STAGE_FILES} 

                ${DB_BASIC_UNIT_TESTS} 
                main.cpp 
                )

add_dependencies( database_basic_tests database_compiler_driver )

# allow access to source folder for tests - NOTE order sensitive here
target_include_directories( database_basic_tests PUBLIC ${BASIC_DIR} )
target_include_directories( database_basic_tests PUBLIC ${DB_API_DIR} )

target_link_libraries( database_basic_tests Boost::filesystem )
target_link_libraries( database_basic_tests Boost::iostreams )
target_link_libraries( database_basic_tests Boost::program_options )
target_link_libraries( database_basic_tests Boost::serialization )
target_link_libraries( database_basic_tests pantor::inja )
target_link_libraries( database_basic_tests nlohmann_json::nlohmann_json )
target_link_libraries( database_basic_tests GTest::gtest GTest::gtest_main )
target_link_libraries( database_basic_tests Common::commonlib ) 

gtest_add_tests( TARGET database_basic_tests SOURCES ${DB_BASIC_UNIT_TESTS}  )

