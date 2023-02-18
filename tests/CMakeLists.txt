include_directories( ${LIBPLINKIO_INCLUDE_DIR} )
include_directories( ${LIBPLINKIO_SOURCE_DIR} )
include_directories( ${LIBCMOCKERY_INCLUDE_DIR} )
include_directories( "." )

add_definitions( -DUNIT_TESTING=1 )

if(MSVC)
    set(PLINKIO_TEST_COMPILE_OPTIONS /Wall /D_CRT_SECURE_NO_WARNINGS /wd4996 /wd5045 /wd4820 /wd4668 /wd4242 /wd4244 /wd4267 /wd4710 /wd4711)
endif()

file(COPY data DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

add_executable( bed_test "bed_test.c" )
target_link_libraries( bed_test libcmockery )
if(WIN32)
    target_link_libraries( bed_test bcrypt )
endif()
target_compile_options( bed_test PRIVATE ${PLINKIO_TEST_COMPILE_OPTIONS})
add_test( bed_test bed_test )


add_executable( bim_test "bim_test.c" "mock.c" )
target_link_libraries( bim_test libcmockery )
target_compile_options( bim_test PRIVATE ${PLINKIO_TEST_COMPILE_OPTIONS})
add_test( bim_test bim_test )


add_executable( fam_test "fam_test.c" "mock.c" )
target_link_libraries( fam_test libcmockery )
target_compile_options( fam_test PRIVATE ${PLINKIO_TEST_COMPILE_OPTIONS})
add_test( fam_test fam_test )


add_executable( map_test "map_test.c" "mock.c" )
target_link_libraries( map_test libcmockery )
target_compile_options( map_test PRIVATE ${PLINKIO_TEST_COMPILE_OPTIONS})
add_test( map_test map_test )


add_executable( ped_test "ped_test.c" )
target_link_libraries( ped_test libcmockery )
if(WIN32)
    target_link_libraries( ped_test bcrypt )
endif()
target_compile_options( ped_test PRIVATE ${PLINKIO_TEST_COMPILE_OPTIONS})
add_test( NAME ped_test COMMAND ped_test WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} )


add_executable( plink_txt_test "plink_txt_test.c" )
target_link_libraries( plink_txt_test libcmockery )
if(WIN32)
    target_link_libraries( plink_txt_test bcrypt )
endif()
target_compile_options( plink_txt_test PRIVATE ${PLINKIO_TEST_COMPILE_OPTIONS})
add_test( NAME plink_txt_test COMMAND plink_txt_test WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} )


add_executable( snp_bit_test "snp_bit_test.c" )
target_link_libraries( snp_bit_test libcmockery )
if(WIN32)
    target_link_libraries( snp_bit_test bcrypt )
endif()
target_compile_options( snp_bit_test PRIVATE ${PLINKIO_TEST_COMPILE_OPTIONS})
add_test( snp_bit_test snp_bit_test )


add_executable( plinkio_test "plinkio_test.c" )
if( NOT DISABLE_STATIC_LIBRARY ) 
    target_link_libraries( plinkio_test libplinkio-static )
else ()
    target_link_libraries( plinkio_test libplinkio )
endif ()
target_compile_options( plinkio_test PRIVATE ${PLINKIO_TEST_COMPILE_OPTIONS})