project( libplinkio )
cmake_minimum_required( VERSION 2.7 )

include( CPack )
set( CPACK_GENERATOR "TGZ" )

set( LIBPLINKIO_VERSION_MAJOR 0 )
set( LIBPLINKIO_VERSION_MINOR 1 )
set( LIBPLINKIO_VERSION_PATCH 0 )

set( LIBCSV_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/libs/libcsv/inc/" )
set( LIBCMOCKERY_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/libs/cmockery/src/google/" )
set( LIBPLINKIO_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/src/plinkio/" )
set( LIBPLINKIO_SOURCE_DIR "${PROJECT_SOURCE_DIR}/src/" )

message( ${LIBCMOCKERY_INCLUDE_DIR} )
enable_testing( )

add_subdirectory( src )
add_subdirectory( libs )
add_subdirectory( tests )

