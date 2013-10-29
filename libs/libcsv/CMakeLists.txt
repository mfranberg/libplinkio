include_directories( "inc/" )

add_library( libcsv STATIC src/libcsv.c )

SET_TARGET_PROPERTIES( libcsv PROPERTIES POSITION_INDEPENDENT_CODE ON )
SET_TARGET_PROPERTIES( libcsv PROPERTIES OUTPUT_NAME csv )
