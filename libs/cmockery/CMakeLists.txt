include_directories( "src/google/" )

add_library( libcmockery "src/cmockery.c" )

if(CMAKE_GENERATOR MATCHES "Visual Studio")
target_compile_options( libcmockery PRIVATE /w)
endif()

target_link_libraries( libcmockery )
SET_TARGET_PROPERTIES( libcmockery PROPERTIES OUTPUT_NAME cmockery )
