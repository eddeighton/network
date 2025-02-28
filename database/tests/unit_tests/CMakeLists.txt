##  Copyright (c) Deighton Systems Limited. 2025. All Rights Reserved.
##  Author: Edward Deighton
##  License: Please see license.txt in the project root folder.

##  Use and copying of this software and preparation of derivative works
##  based upon this software are permitted. Any copy of this software or
##  of any derivative work must include the above copyright notice, this
##  paragraph and the one after it.  Any distribution of this software or
##  derivative works must comply with all applicable laws.

##  This software is made available AS IS, and COPYRIGHT OWNERS DISCLAIMS
##  ALL WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE
##  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
##  PURPOSE, AND NOTWITHSTANDING ANY OTHER PROVISION CONTAINED HEREIN, ANY
##  LIABILITY FOR DAMAGES RESULTING FROM THE SOFTWARE OR ITS USE IS
##  EXPRESSLY DISCLAIMED, WHETHER ARISING IN CONTRACT, TORT (INCLUDING
##  NEGLIGENCE) OR STRICT LIABILITY, EVEN IF COPYRIGHT OWNERS ARE ADVISED
##  OF THE POSSIBILITY OF SUCH DAMAGES.

set( DB_UNIT_TESTS
	schema_grammar_tests.cpp
	component_type_tests.cpp
	manifest_tests.cpp
	database_tests.cpp
	main_test_runner.cpp
)

enable_testing()

add_executable( db_tests ${DB_UNIT_TESTS} main_test_runner.cpp )

target_link_libraries( db_tests Boost::filesystem )
target_link_libraries( db_tests Boost::iostreams )
target_link_libraries( db_tests Boost::atomic )
target_link_libraries( db_tests Boost::program_options )
target_link_libraries( db_tests Boost::serialization )
target_link_libraries( db_tests GTest::gtest GTest::gtest_main )
target_link_libraries( db_tests database_compiler)

gtest_add_tests( TARGET db_tests SOURCES ${DB_UNIT_TESTS} )

