include_directories( ${LIBPLINKIO_INCLUDE_DIR} )
include_directories( ${LIBPLINKIO_SOURCE_DIR} )
include_directories( ${LIBCMOCKERY_INCLUDE_DIR} )
include_directories( ${LIBCSV_INCLUDE_DIR} )
include_directories( "." )

add_definitions( -DUNIT_TESTING=1 )

add_executable( bed_test "bed_test.c" )
target_link_libraries( bed_test libcmockery libcsv )
add_test( bed_test bed_test )

add_executable( bim_test "bim_test.c" "mock.c" )
target_link_libraries( bim_test libcmockery libcsv )
add_test( bim_test bim_test )

add_executable( fam_test "fam_test.c" "mock.c" )
target_link_libraries( fam_test libcmockery libcsv )
add_test( fam_test fam_test )

add_executable( plinkio_test "plinkio_test.c" )
target_link_libraries( plinkio_test libcsv libplinkio-static )
