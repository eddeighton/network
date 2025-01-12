
#########################################
#########################################
# resolve custom clang_eg build of llvm
#set( CLANG_EG_INSTALL_PATH /usr/local/clangeg )

#find_path( LLVM_DIR NAMES LLVMConfig.cmake PATHS ${CLANG_EG_INSTALL_PATH}/lib/cmake/llvm/ REQUIRED NO_DEFAULT_PATH)
#find_path( CLANG_DIR NAMES ClangConfig.cmake PATHS ${CLANG_EG_INSTALL_PATH}/lib/cmake/clang/ REQUIRED NO_DEFAULT_PATH)

find_package( LLVM REQUIRED CONFIG )
find_package( Clang REQUIRED CONFIG )

#set(LLVM_LINK_COMPONENTS support)

function( include_clang targetname )
	target_compile_definitions( ${targetname} PUBLIC ${LLVM_DEFINITIONS} )
	target_compile_definitions( ${targetname} PUBLIC ${CLANG_DEFINITIONS} )

	target_include_directories( ${targetname} PUBLIC ${LLVM_INCLUDE_DIRS} )
	target_include_directories( ${targetname} PUBLIC ${CLANG_INCLUDE_DIRS} )

endfunction( include_clang )

function( link_clang targetname )
	include_clang( ${targetname} )

	target_link_directories( ${targetname} PUBLIC ${LLVM_LIB_DIR} )
	target_link_directories( ${targetname} PUBLIC ${CLANG_LIB_DIR} )

    #target_link_libraries( ${targetname} ${llvm_libs} )
	target_link_libraries( ${targetname} clangTooling )
	
endfunction( link_clang )

function( link_clang_tooling targetname )
	include_clang( ${targetname} )

	target_link_directories( ${targetname} PUBLIC ${LLVM_LIB_DIR} )
	target_link_directories( ${targetname} PUBLIC ${CLANG_LIB_DIR} )

	llvm_map_components_to_libnames( llvm_tool_libs Support FrontendOpenMP )
	#set(llvm_tool_libs)
	Message(STATUS "llvm_tool_libs= ${llvm_tool_libs}")

	#target_link_libraries( ${targetname} ${llvm_tool_libs} )

    	target_link_libraries( ${targetname} 
	
		clangAST
		clangASTMatchers
		clangBasic
		clangFrontend
		clangSerialization
		clangTooling
	)
	
endfunction( link_clang_tooling )

function( link_orc targetname )
	include_clang( ${targetname} )

	target_link_directories( ${targetname} PUBLIC ${LLVM_LIB_DIR} )
	target_link_directories( ${targetname} PUBLIC ${CLANG_LIB_DIR} )

	llvm_map_components_to_libnames( llvm_orc_libs core support codegen irreader x86codegen X86TargetMCA X86Desc X86Info LTO orcjit )
	#set(llvm_libs)
	Message(STATUS "llvm_orc_libs= ${llvm_orc_libs}")
	target_link_libraries( ${targetname} ${llvm_orc_libs} )
	
endfunction( link_orc )

function( import_clang targetname )
	include_clang( ${targetname} )
endfunction( import_clang )


