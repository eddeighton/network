


set( COMMON_UNIT_TESTS
     report_tests.cpp
     )

add_executable( report_tests ${COMMON_UNIT_TESTS} )
target_link_libraries(report_tests reportlib)
target_link_libraries(report_tests GTest::gtest GTest::gtest_main)

gtest_add_tests( TARGET report_tests SOURCES ${COMMON_UNIT_TESTS} )

