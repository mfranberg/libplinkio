include_directories( "." )
include_directories( ${LIBCSV_INCLUDE_DIR} )

file( GLOB_RECURSE SRC_LIST "*.c" "." )
file( GLOB_RECURSE HEADERS "*.h" "plinkio/" )

if( NOT DISABLE_SHARED_LIBRARY ) 
    add_library( libplinkio SHARED ${SRC_LIST} )
    target_link_libraries( libplinkio libcsv )
    SET_TARGET_PROPERTIES( libplinkio PROPERTIES OUTPUT_NAME plinkio )
    install( TARGETS libplinkio DESTINATION lib )
endif( )

if( NOT DISABLE_STATIC_LIBRARY ) 
    add_library( libplinkio-static STATIC ${SRC_LIST} )
    target_link_libraries( libplinkio-static libcsv )
endif( )

SET_TARGET_PROPERTIES( libplinkio-static PROPERTIES OUTPUT_NAME plinkio )

install( FILES ${HEADERS} DESTINATION include/plinkio )
