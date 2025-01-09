
set( COMMON_UNIT_TESTS
     mptf_test.cpp
     mptfo_test.cpp
     thread_id_test.cpp
     )

add_executable( mptfo_tests ${COMMON_UNIT_TESTS} )
target_link_libraries(mptfo_tests GTest::gtest GTest::gtest_main)
target_link_libraries(mptfo_tests Boost::serialization )


gtest_add_tests( TARGET mptfo_tests SOURCES ${COMMON_UNIT_TESTS} )

