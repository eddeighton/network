cmake_minimum_required(VERSION 3.2)

#args: 
#DBNAME <name>
#DBCOMPILER <compiler> 
#LIB <lib dir> 
#TEMPLATE <template dir> 
#DATA <data dir> 
#API <api dir> 
#SRC <src dir> 
#SCHEMAS <schema file list> 
#STAGES <stage name list>
macro( MegaDatabase )

	set( input_args_list "${ARGN}" )

	set( DBName  )
	set( DBCompilerPath  )
	set( LibPath  )
	set( TemplatesPath  )
	set( DataPath  )
	set( APIPath  )
	set( SRCPath )
	set( Schemas )
	set( Stages )

	set( ACTIVE_LIST )
	#Message( STATUS "input_args_list  = ${input_args_list}" )

	foreach(loop_var IN LISTS input_args_list)

		if( ${loop_var} MATCHES "DBNAME" )
			set( ACTIVE_LIST DBNAME )
		elseif( ${loop_var} MATCHES "DBCOMPILER" )
			set( ACTIVE_LIST DBCOMPILER )
		elseif( ${loop_var} MATCHES "LIB" )
			set( ACTIVE_LIST LIB )
		elseif( ${loop_var} MATCHES "TEMPLATE" )
			set( ACTIVE_LIST TEMPLATE )
		elseif( ${loop_var} MATCHES "DATA" )
			set( ACTIVE_LIST DATA )
		elseif( ${loop_var} MATCHES "API" )
			set( ACTIVE_LIST API )
		elseif( ${loop_var} MATCHES "SRC" )
			set( ACTIVE_LIST SRC )
		elseif( ${loop_var} MATCHES "SCHEMAS" )
			set( ACTIVE_LIST SCHEMAS )
		elseif( ${loop_var} MATCHES "STAGES" )
			set( ACTIVE_LIST STAGES )
		else()
			if( ${ACTIVE_LIST} MATCHES "DBNAME" )
				set(DBName ${loop_var})
			elseif( ${ACTIVE_LIST} MATCHES "DBCOMPILER" )
				set(DBCompilerPath ${loop_var})
			elseif( ${ACTIVE_LIST} MATCHES "LIB" )
				set(LibPath ${loop_var})
			elseif( ${ACTIVE_LIST} MATCHES "TEMPLATE" )
				set(TemplatesPath ${loop_var})
			elseif( ${ACTIVE_LIST} MATCHES "DATA" )
				set(DataPath ${loop_var})
			elseif( ${ACTIVE_LIST} MATCHES "API" )
				set(APIPath ${loop_var})
			elseif( ${ACTIVE_LIST} MATCHES "SRC" )
				set(SRCPath ${loop_var})
			elseif( ${ACTIVE_LIST} MATCHES "SCHEMAS" )
				list(APPEND Schemas ${loop_var})
			elseif( ${ACTIVE_LIST} MATCHES "STAGES" )
				list(APPEND Stages ${loop_var})
			endif()
		endif()
	endforeach()

	set( DATABASE_SOURCE_FILES_HEADERS

		${LibPath}/api/archive.hpp
		${LibPath}/api/component_info.hpp
		${LibPath}/api/component_type.hpp
		${LibPath}/api/data_pointer.hpp
		${LibPath}/api/data_variant.hpp
		${LibPath}/api/exception.hpp
		${LibPath}/api/file_header.hpp
		${LibPath}/api/file.hpp
		${LibPath}/api/file_store.hpp
		${LibPath}/api/file_system.hpp
		${LibPath}/api/generics.hpp
		${LibPath}/api/loader.hpp
		${LibPath}/api/manifest_data.hpp
		${LibPath}/api/object.hpp
		${LibPath}/api/object_info.hpp
		${LibPath}/api/object_loader.hpp
		${LibPath}/api/serialisation.hpp
		${LibPath}/api/storer.hpp
	)

	set( DATABASE_SOURCE_FILES_SRC

		${LibPath}/src/archive.cpp
		${LibPath}/src/file.cpp	
		${LibPath}/src/loader.cpp
		${LibPath}/src/object.cpp
		${LibPath}/src/storer.cpp
	)
	set( DATABASE_SOURCE_FILES ${DATABASE_SOURCE_FILES_HEADERS} ${DATABASE_SOURCE_FILES_SRC} )

	set( DATABASE_COPIED_FILES_HPP

		${APIPath}/database/archive.hpp
		${APIPath}/database/component_info.hpp
		${APIPath}/database/component_type.hpp
		${APIPath}/database/data_pointer.hpp
		${APIPath}/database/data_variant.hpp
		${APIPath}/database/exception.hpp
		${APIPath}/database/file_header.hpp
		${APIPath}/database/file.hpp
		${APIPath}/database/file_store.hpp
		${APIPath}/database/file_system.hpp
		${APIPath}/database/generics.hpp
		${APIPath}/database/loader.hpp
		${APIPath}/database/manifest_data.hpp
		${APIPath}/database/object.hpp
		${APIPath}/database/object_info.hpp
		${APIPath}/database/object_loader.hpp
		${APIPath}/database/serialisation.hpp
		${APIPath}/database/storer.hpp
	)

	set( DATABASE_COPIED_FILES_CPP

		${SRCPath}/database/archive.cpp
		${SRCPath}/database/file.cpp
		${SRCPath}/database/loader.cpp
		${SRCPath}/database/object.cpp
		${SRCPath}/database/storer.cpp
	)

	set( DATABASE_GENERATED_FILES_HXX

		${APIPath}/database/data.hxx
		${APIPath}/database/environment.hxx
		${APIPath}/database/file_info.hxx
		${APIPath}/database/manifest.hxx
	)

	set( DATABASE_GENERATED_FILES_CXX

		${SRCPath}/database/data.cxx
		${SRCPath}/database/environment.cxx
		${SRCPath}/database/file_info.cxx
		${SRCPath}/database/manifest.cxx
	)

	set( BASIC_DATABASE_GENERATED_DATA
		${DataPath}/data.json
		${DataPath}/stages.json
	)
	foreach( stage ${Stages})
		list(APPEND DATABASE_GENERATED_FILES_HXX ${APIPath}/database/${stage}.hxx )
		list(APPEND DATABASE_GENERATED_FILES_CXX ${SRCPath}/database/${stage}.cxx )
		list(APPEND BASIC_DATABASE_GENERATED_DATA ${DataPath}/${stage}.json )
	endforeach()

	set( DATABASE_COPIED_FILES ${DATABASE_COPIED_FILES_HPP} ${DATABASE_COPIED_FILES_CPP} )
	set( DATABASE_GENERATED_FILES ${DATABASE_GENERATED_FILES_HXX} ${DATABASE_GENERATED_FILES_CXX} )

	#Message( STATUS "Executing database compiler: ${DBCompilerPath} for Schemas: ${Schemas} and Stages:${Stages} with lib: ${LibPath} templates: ${TemplatesPath} api: ${APIPath} src: ${SRCPath} data: ${DataPath}" )

	add_custom_command( 
		OUTPUT 
		${DATABASE_COPIED_FILES} 
		${DATABASE_GENERATED_FILES} 

		COMMAND ${DBCompilerPath}
			--lib       ${LibPath}
			--templates ${TemplatesPath}
			--api       ${APIPath}/database
			--src       ${SRCPath}/database
			--data      ${DataPath}
			--stash     ${CMAKE_CURRENT_BINARY_DIR}/stash
			--input     ${Schemas}

		BYPRODUCTS ${BASIC_DATABASE_GENERATED_DATA}
		DEPENDS ${Schemas} ${DBCompilerPath}
		COMMENT "Generating Database" 
	)


	# CMAKE DAY Attempt 1 - 21 Oct 2023
	#set(headerCopyCommands)
	#set(srcCopyCommands)

	#foreach(headerFile ${DATABASE_SOURCE_FILES_HEADERS})
	#list(APPEND headerCopyCommands
	#	COMMAND ${CMAKE_COMMAND} -E copy_if_different ${headerFile} ${APIPath}/database/)
	#endforeach()

	#foreach(srcFile ${DATABASE_SOURCE_FILES_SRC})
	#list(APPEND srcCopyCommands
	#	COMMAND ${CMAKE_COMMAND} -E copy_if_different ${srcFile} ${SRCPath}/database/)
	#endforeach()

	#add_custom_target( UpdateDB_${DBName}
	#	COMMENT "Copying database lib files to ${APIPath}/database/ and ${SRCPath}/database/"
	#	Message( STATUS "Copying database lib files to ${APIPath}/database/ and ${SRCPath}/database/" )
	#	
	#	${headerCopyCommands}
	#	${srcCopyCommands}

	#)

	# Attempt 2
	add_custom_target( UpdateDB_${DBName}

		# OUTPUT 
		# ${DATABASE_COPIED_FILES} 
		# ${DATABASE_GENERATED_FILES} 

		COMMAND ${DBCompilerPath}
			--lib       ${LibPath}
			--templates ${TemplatesPath}
			--api       ${APIPath}/database
			--src       ${SRCPath}/database
			--data      ${DataPath}
			--stash     ${CMAKE_CURRENT_BINARY_DIR}/stash
			--input     ${Schemas}

		BYPRODUCTS 
		DEPENDS ${Schemas}
		COMMENT "Generating Database via UpdateDB_${DBName} from ${LibPath} to ${APIPath}/database" 
	)


endmacro()
