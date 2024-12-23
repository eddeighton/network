
set( UNIT_TESTS
     basic_tests.cpp
 )

add_executable( controller_tests ${UNIT_TESTS} )
target_link_libraries(controller_tests libcontroller)
target_link_libraries(controller_tests commonlib)
target_link_libraries(controller_tests GTest::gtest GTest::gtest_main)

gtest_add_tests( TARGET controller_tests SOURCES ${UNIT_TESTS} )

