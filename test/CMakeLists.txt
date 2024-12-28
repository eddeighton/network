
set( COMMON_UNIT_TESTS
     basic_tests.cpp)

add_executable( service_tests ${COMMON_UNIT_TESTS} )
target_link_libraries(service_tests service)
target_link_libraries(service_tests commonlib)
target_link_libraries(service_tests Boost::fiber)
target_link_libraries(service_tests GTest::gtest GTest::gtest_main)

gtest_add_tests( TARGET service_tests SOURCES ${COMMON_UNIT_TESTS} )

