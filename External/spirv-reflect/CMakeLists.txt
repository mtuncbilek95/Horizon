cmake_minimum_required(VERSION 3.26)

include(FetchContent)

# Make URL variable
if(NOT DEFINED SPIRV_REFLECT_URL)
	set(SPIRV_REFLECT_URL https://github.com/KhronosGroup/SPIRV-Reflect.git)
endif()

# SPIRV reflect
FetchContent_Declare(
    spirv-reflect
	EXCLUDE_FROM_ALL
    GIT_REPOSITORY ${SPIRV_REFLECT_URL}
    GIT_TAG main
	GIT_SHALLOW TRUE
)

set(SPIRV_REFLECT_BUILD_TOOLS OFF CACHE BOOL "Disable SPIRV-Reflect tools" FORCE)
set(SPIRV_REFLECT_EXECUTABLE OFF CACHE BOOL "Disable spirv-reflect executable" FORCE)
set(SPIRV_REFLECT_INSTALL OFF CACHE BOOL "Disable spirv-reflect install" FORCE)
set(SPIRV_REFLECT_PP OFF CACHE BOOL "Disable spirv-reflect-pp" FORCE)
set(SPIRV_REFLECT_STATIC_LIB ON)

FetchContent_MakeAvailable(spirv-reflect)