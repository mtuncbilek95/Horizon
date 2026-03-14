cmake_minimum_required(VERSION 3.26 FATAL_ERROR)

include(FetchContent)

# Make URL variable
if(NOT DEFINED IMGUI_URL)
	set(IMGUI_URL https://github.com/ocornut/imgui.git)
endif()

# IMGUI Library
FetchContent_Declare(
	imgui
	EXCLUDE_FROM_ALL
	GIT_REPOSITORY ${IMGUI_URL}
	GIT_TAG docking
	GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(imgui)

file(GLOB IMGUI_FILES "${imgui_SOURCE_DIR}/*.cpp" "${imgui_SOURCE_DIR}/*.h" "${imgui_SOURCE_DIR}/backends/imgui_impl_vulkan.h" 
"${imgui_SOURCE_DIR}/backends/imgui_impl_vulkan.cpp" "${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.h"
"${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp")
source_group(TREE "${imgui_SOURCE_DIR}" FILES ${IMGUI_FILES})
add_library(Imgui STATIC ${IMGUI_FILES})

target_include_directories(Imgui PUBLIC ${imgui_SOURCE_DIR})

include(SetupVulkan)
SetupVulkan(Imgui)

include(SetupGLFW)
SetupGLFW(Imgui)