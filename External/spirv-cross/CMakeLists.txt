cmake_minimum_required(VERSION 3.26)

include(FetchContent)

# Make URL variable
if(NOT DEFINED SPIRV_CROSS_URL)
	set(SPIRV_CROSS_URL https://github.com/KhronosGroup/SPIRV-Cross.git)
endif()

# SPIRV reflect
FetchContent_Declare(
    SPIRV-Cross
	EXCLUDE_FROM_ALL
    GIT_REPOSITORY ${SPIRV_CROSS_URL}
    GIT_TAG main
	GIT_SHALLOW TRUE
)

#set(SPIRV_CROSS_STATIC ON)
#set(SPIRV_CROSS_ENABLE_GLSL ON)
#set(SPIRV_CROSS_ENABLE_HLSL ON)
#set(SPIRV_CROSS_ENABLE_MSL ON)
#set(SPIRV_CROSS_ENABLE_CPP ON)
#set(SPIRV_CROSS_ENABLE_UTIL ON)
#set(SPIRV_CROSS_FORCE_STL_TYPES ON)
FetchContent_MakeAvailable(SPIRV-Cross)