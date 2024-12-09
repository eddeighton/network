
set( COMMON_UNIT_TESTS
     ${CMAKE_CURRENT_SOURCE_DIR}/astar_tests.cpp
     ${CMAKE_CURRENT_SOURCE_DIR}/angle_tests.cpp
     ${CMAKE_CURRENT_SOURCE_DIR}/file_tests.cpp
     ${CMAKE_CURRENT_SOURCE_DIR}/grammar_tests.cpp
     ${CMAKE_CURRENT_SOURCE_DIR}/inplace_allocator_tests.cpp
     ${CMAKE_CURRENT_SOURCE_DIR}/math_tests.cpp
     # ${CMAKE_CURRENT_SOURCE_DIR}/preprocessor_tests.cpp
     ${CMAKE_CURRENT_SOURCE_DIR}/rounding_tests.cpp
     ${CMAKE_CURRENT_SOURCE_DIR}/tuple_tests.cpp
     ${CMAKE_CURRENT_SOURCE_DIR}/scheduler_tests.cpp
     ${CMAKE_CURRENT_SOURCE_DIR}/escape_tests.cpp
     ${CMAKE_CURRENT_SOURCE_DIR}/stash_tests.cpp
     ${CMAKE_CURRENT_SOURCE_DIR}/process_tests.cpp )

add_executable( common_tests ${COMMON_UNIT_TESTS} )
target_link_libraries(common_tests commonlib)
target_link_libraries(common_tests GTest::gtest GTest::gtest_main)

gtest_add_tests( TARGET common_tests SOURCES ${COMMON_UNIT_TESTS} )

