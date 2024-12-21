
set( PIPELINEUNIT_TESTS
     ${CMAKE_CURRENT_SOURCE_DIR}/pipeline_tests.cpp
  )

add_executable( pipeline_tests ${PIPELINEUNIT_TESTS} )
target_link_libraries(pipeline_tests pipeline)
target_link_libraries(pipeline_tests GTest::gtest GTest::gtest_main)

gtest_add_tests( TARGET pipeline_tests SOURCES ${PIPELINEUNIT_TESTS} )

