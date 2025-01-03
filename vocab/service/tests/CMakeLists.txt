
set( COMMON_UNIT_TESTS
     mptfo_test.cpp
     )

add_executable( mptfo_tests ${COMMON_UNIT_TESTS} )
target_link_libraries(mptfo_tests GTest::gtest GTest::gtest_main)

gtest_add_tests( TARGET mptfo_tests SOURCES ${COMMON_UNIT_TESTS} )

