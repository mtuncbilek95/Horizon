function(SetupConfig)
    add_compile_definitions(
        $<$<CONFIG:Debug>:HORIZON_DEBUG=1>
        $<$<CONFIG:Debug>:HORIZON_MEMORY_TRACKING=1>
        $<$<CONFIG:Release>:HORIZON_RELEASE=1>
    )

add_compile_definitions(HORIZON_RESOURCE_DIR="${CMAKE_SOURCE_DIR}/Resources")

endfunction()