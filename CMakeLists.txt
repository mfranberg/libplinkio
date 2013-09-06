project( libplinkio )
cmake_minimum_required( VERSION 2.8.9 )

set( LIBPLINKIO_VERSION_MAJOR 0 )
set( LIBPLINKIO_VERSION_MINOR 1 )
set( LIBPLINKIO_VERSION_PATCH 0 )

set( CPACK_SOURCE_GENERATOR "TGZ" )
set( CPACK_SOURCE_PACKAGE_FILE_NAME
    "${CMAKE_PROJECT_NAME}-${LIBPLINKIO_VERSION_MAJOR}.${LIBPLINKIO_VERSION_MINOR}.${LIBPLINKIO_VERSION_PATCH}" )
set( CPACK_SOURCE_IGNORE_FILES "build*;/.git/;log$;gitignore$;/autom4te.cache/;~$;${CPACK_SOURCE_IGNORE_FILES}" )
include( CPack )

set( LIBCSV_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/libs/libcsv/inc/" )
set( LIBCMOCKERY_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/libs/cmockery/src/google/" )
set( LIBPLINKIO_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/src/plinkio/" )
set( LIBPLINKIO_SOURCE_DIR "${PROJECT_SOURCE_DIR}/src/" )

option( DISABLE_TESTS "Disables building tests." OFF )

add_subdirectory( src )
add_subdirectory( libs )

if( NOT DISABLE_TESTS )
    enable_testing( )
    add_subdirectory( tests )
endif( )
