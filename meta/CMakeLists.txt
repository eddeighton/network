
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

