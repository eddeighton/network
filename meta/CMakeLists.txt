
include( ${CMAKE_SOURCE_DIR}/database/database.cmake )

set(META_DB_PATH ${CMAKE_CURRENT_SOURCE_DIR}/db)

MegaDatabase( 
    DBNAME      meta
    DBCOMPILER  database_compiler_driver
    LIB         ${CMAKE_SOURCE_DIR}/database/compiler/lib 
    TEMPLATE    ${CMAKE_SOURCE_DIR}/database/compiler/templates
    DATA        ${META_DB_PATH}/data/
    API         ${META_DB_PATH}
    SRC         ${META_DB_PATH}
    SCHEMAS     ${CMAKE_CURRENT_SOURCE_DIR}/schema.dbs
    STAGES      AnalysisStage 
)

add_subdirectory( pipeline )
add_subdirectory( driver )

set(SERVICE_INTERFACES ${CMAKE_CURRENT_SOURCE_DIR}/test.interface.hpp)

set(GENERATED_INTERFACE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/test.cxx)

add_custom_command(
    OUTPUT ${GENERATED_INTERFACE_FILES}

    COMMAND meta
        --src_dir ${CMAKE_SOURCE_DIR}
        --build_dir ${CMAKE_BINARY_DIR}
        --stash_dir ${CMAKE_CURRENT_BINARY_DIR}/stash
        --templates ${CMAKE_CURRENT_SOURCE_DIR}/templates
        --pipeline "$<TARGET_FILE:meta_pipeline>"
        --interfaces \"${SERVICE_INTERFACES}\"
 
    BYPRODUCTS # ${GENERATED_INTERFACE_FILES}
    DEPENDS ${SERVICE_INTERFACES}
    COMMENT "Running meta pipeline"
)

add_library( meta_service_interface
    ${GENERATED_INTERFACE_FILES}
)

