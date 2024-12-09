##  Copyright (c) Deighton Systems Limited. 2022. All Rights Reserved.
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

cmake_minimum_required( VERSION 3.1...3.16 )

set( MEGA_DATABASE_TEMPLATES
    templates/data.cxx.jinja
    templates/data.hxx.jinja
    templates/environment.cxx.jinja
    templates/environment.hxx.jinja
    templates/file_info.cxx.jinja
    templates/file_info.hxx.jinja
    templates/manifest.cxx.jinja
    templates/manifest.hxx.jinja
    templates/view.cxx.jinja
    templates/view.hxx.jinja
)

set( MEGA_DATABASE_COMPILER
    native_types.hpp
    generator.hpp
    generator.cpp
    grammar.hpp
    grammar.cpp
    json_converter.hpp
    json_converter.cpp
    model.hpp
    model.cpp
	)
	
set( MEGA_DATABASE_LIB_HEADERS
    lib/api/archive.hpp
    lib/api/component_info.hpp
    lib/api/component_type.hpp
    lib/api/data_pointer.hpp
    lib/api/data_variant.hpp
    lib/api/directories.hpp
    lib/api/exception.hpp
    lib/api/file_header.hpp
    lib/api/file_store.hpp
    lib/api/file_system.hpp
    lib/api/file.hpp
    lib/api/generics.hpp
    lib/api/loader.hpp
    lib/api/manifest_data.hpp
    lib/api/object_info.hpp
    lib/api/object_loader.hpp
    lib/api/object.hpp
    lib/api/serialisation.hpp
    lib/api/sources.hpp
    lib/api/storer.hpp
)

set( MEGA_DATABASE_LIB_SRC
    lib/src/archive.cpp
    lib/src/file.cpp
    lib/src/loader.cpp
    lib/src/object.cpp
    lib/src/storer.cpp
)

add_library( database_lib INTERFACE ${MEGA_DATABASE_LIB_HEADERS} ${MEGA_DATABASE_LIB_SRC} )

add_library( database_compiler ${MEGA_DATABASE_COMPILER} )

target_link_libraries( database_compiler Boost::filesystem )
target_link_libraries( database_compiler Boost::iostreams )
target_link_libraries( database_compiler pantor::inja )
target_link_libraries( database_compiler nlohmann_json::nlohmann_json )
target_link_libraries( database_compiler Common::commonlib ) 

add_executable( database_compiler_driver main.cpp )

target_link_libraries( database_compiler_driver Boost::filesystem )
target_link_libraries( database_compiler_driver Boost::program_options )
target_link_libraries( database_compiler_driver Common::commonlib ) 
target_link_libraries( database_compiler_driver database_compiler )

