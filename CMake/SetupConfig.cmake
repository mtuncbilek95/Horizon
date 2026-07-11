function(SetupConfig)
    add_compile_definitions(HORIZON_RESOURCE_DIR="${CMAKE_SOURCE_DIR}/Resources")

    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_compile_definitions(HORIZON_DEBUG)
    endif()

    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        add_compile_definitions(HORIZON_RELEASE)
    endif()
endfunction()