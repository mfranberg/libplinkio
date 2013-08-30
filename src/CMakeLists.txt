include_directories( "plinkio/" )
include_directories( ${LIBCSV_INCLUDE_DIR} )

file( GLOB_RECURSE SRC_LIST "*.c" "." )
add_library( libplinkio SHARED ${SRC_LIST} )
target_link_libraries( libplinkio libcsv )

add_library( libplinkio-static SHARED ${SRC_LIST} )
target_link_libraries( libplinkio-static libcsv )

SET_TARGET_PROPERTIES( libplinkio PROPERTIES OUTPUT_NAME plinkio )
SET_TARGET_PROPERTIES( libplinkio-static PROPERTIES OUTPUT_NAME plinkio )

file( GLOB_RECURSE HEADERS "*.h" "plinkio/" )
install( TARGETS libplinkio DESTINATION lib )
install( FILES ${HEADERS} DESTINATION include/plinkio )
