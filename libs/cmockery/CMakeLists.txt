include_directories( "src/google/" )

add_library( libcmockery "src/cmockery.c" )

target_link_libraries( libcmockery )
SET_TARGET_PROPERTIES( libcmockery PROPERTIES OUTPUT_NAME cmockery )
