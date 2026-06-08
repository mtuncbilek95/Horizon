cmake_minimum_required(VERSION 3.26 FATAL_ERROR)

include(SetupGLFW)

file(GLOB_RECURSE SUBDIRECTORIES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp" "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${SUBDIRECTORIES})

add_library(Runtime STATIC ${SUBDIRECTORIES})

target_include_directories(Runtime PRIVATE "${CMAKE_SOURCE_DIR}/Source")

target_link_libraries(Runtime
	PUBLIC
		glm::glm glfw
)
SetupGLFW(Runtime)

target_link_libraries(Runtime
	PUBLIC
		WinPixEventRuntime
)
target_compile_definitions(Runtime 
	PUBLIC 
		$<$<CONFIG:Debug>:USE_PIX>
)

target_precompile_headers(Runtime PUBLIC "pch.h")

if(WIN32)
    include(SetupWindows)
    SetupWin32(Runtime)
	SetupD3D(Runtime)
	target_link_libraries(Runtime PUBLIC D3D12MemoryAllocator)
endif()