
set( COMMON_UNIT_TESTS
     astar_tests.cpp
     angle_tests.cpp
     file_tests.cpp
     grammar_tests.cpp
     inplace_allocator_tests.cpp
     math_tests.cpp
     rounding_tests.cpp
     tuple_tests.cpp
     scheduler_tests.cpp
     escape_tests.cpp
     stash_tests.cpp
     process_tests.cpp )

add_executable( common_tests ${COMMON_UNIT_TESTS} )
target_link_libraries(common_tests commonlib)
target_link_libraries(common_tests GTest::gtest GTest::gtest_main)

gtest_add_tests( TARGET common_tests SOURCES ${COMMON_UNIT_TESTS} )

