project( libplinkio )
cmake_minimum_required( VERSION 3.20.0 )
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set( LIBPLINKIO_VERSION_MAJOR 0 )
set( LIBPLINKIO_VERSION_MINOR 3 )
set( LIBPLINKIO_VERSION_PATCH 1 )

set( CPACK_SOURCE_GENERATOR "TGZ" )
set( CPACK_SOURCE_PACKAGE_FILE_NAME
    "${CMAKE_PROJECT_NAME}-${LIBPLINKIO_VERSION_MAJOR}.${LIBPLINKIO_VERSION_MINOR}.${LIBPLINKIO_VERSION_PATCH}" )
set( CPACK_SOURCE_IGNORE_FILES "build*;/.git/;log$;gitignore$;/autom4te.cache/;~$;${CPACK_SOURCE_IGNORE_FILES}" )
include( CPack )

set( LIBCMOCKERY_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/libs/cmockery/src/google/" )
set( LIBPLINKIO_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/src/plinkio/" "${PROJECT_BINARY_DIR}/src/plinkio/" "${PROJECT_BINARY_DIR}/src/" )
set( LIBPLINKIO_SOURCE_DIR "${PROJECT_SOURCE_DIR}/src/" )

option( DISABLE_TESTS "Disables building tests." OFF )
option( DISABLE_SHARED_LIBS "Disable building shared library." OFF )
option( DISABLE_STATIC_LIBS "Disable building static library." OFF )
option( DISABLE_INSTALL_HEADERS "Disable installing header files." OFF )

add_subdirectory( src )
add_subdirectory( libs )

if( NOT DISABLE_TESTS )
    enable_testing( )
    add_subdirectory( tests )
endif( )
