if(NOT DEFINED SOL2_URL)
	set(SOL2_URL https://github.com/ThePhD/sol2.git)
endif()

FetchContent_Declare(
	sol2
	EXCLUDE_FROM_ALL
	GIT_REPOSITORY ${SOL2_URL}
	GIT_TAG v3.3.0
	GIT_SHALLOW TRUE
	SOURCE_SUBDIR DoNotBuild
)

FetchContent_MakeAvailable(sol2)

add_library(sol2 INTERFACE)
target_include_directories(sol2 INTERFACE "${sol2_SOURCE_DIR}/include")
target_link_libraries(sol2 INTERFACE lua)
target_compile_definitions(sol2 INTERFACE
	SOL_USING_CXX_LUA=1
	$<$<CONFIG:Debug>:SOL_ALL_SAFETIES_ON=1>
)
target_compile_options(sol2 INTERFACE $<$<CXX_COMPILER_ID:MSVC>:/bigobj>)