include_directories( "inc/" )

add_library( libcsv src/libcsv.c )

SET_TARGET_PROPERTIES( libcsv PROPERTIES OUTPUT_NAME csv )
