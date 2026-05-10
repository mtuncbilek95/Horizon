cmake_minimum_required(VERSION 3.20 FATAL_ERROR)

include(SetupVulkan)
include(FindSPIRV)
include(SetupGLFW)

file(GLOB_RECURSE SUBDIRECTORIES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp" "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${SUBDIRECTORIES})

add_library(Runtime 
	STATIC 
		${SUBDIRECTORIES}
)

target_include_directories(Runtime 
	PRIVATE 
		"${CMAKE_SOURCE_DIR}/Source"
)

target_link_libraries(Runtime 
	PUBLIC 
		glm::glm magic_enum stb assimp glfw EnTT::EnTT 
		spirv-reflect-static spirv-cross-glsl spirv-cross-c spirv-cross-core
		nlohmann_json::nlohmann_json
)

target_precompile_headers(Runtime 
	PUBLIC 
		"${CMAKE_SOURCE_DIR}/Source/Runtime/PCH.h"
)

DefineGlmMacros(Runtime)
AddStbMacros(Runtime)
SetupVulkan(Runtime)
SetupGLFW(Runtime)
BindSpirv(Runtime)

if(WIN32)
    include(SetupWindows)
    SetupWin32(Runtime)
endif()

install(TARGETS Runtime
    ARCHIVE DESTINATION "lib"
)