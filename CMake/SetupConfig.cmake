function(SetupConfig)
    add_compile_definitions(
        $<$<CONFIG:Debug>:HORIZON_DEBUG=1>
        $<$<CONFIG:Debug>:HORIZON_MEMORY_TRACKING=1>
        $<$<CONFIG:Release>:HORIZON_RELEASE=1>
    )

	add_compile_definitions(HORIZON_RESOURCE_DIR="${CMAKE_SOURCE_DIR}/Resources")
	add_compile_definitions(HORIZON_EXAMPLE_DIR="${CMAKE_SOURCE_DIR}/ExampleProject")

	if(MSVC)
		add_compile_options(
			$<$<CONFIG:Release>:/Ob3>
			$<$<CONFIG:Release>:/GL>
			$<$<CONFIG:Release>:/Gw>
			$<$<CONFIG:Release>:/arch:AVX2>
		)

		add_link_options(
			$<$<CONFIG:Release>:/LTCG>
			$<$<CONFIG:Release>:/OPT:REF>
			$<$<CONFIG:Release>:/OPT:ICF>
		)
	endif()

endfunction()