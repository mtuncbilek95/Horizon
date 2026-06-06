cmake_minimum_required(VERSION 3.26 FATAL_ERROR)

include(SetupGLFW)

file(GLOB_RECURSE SUBDIRECTORIES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp" "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${SUBDIRECTORIES})

add_library(Engine STATIC ${SUBDIRECTORIES})

target_include_directories(Engine PRIVATE "${CMAKE_SOURCE_DIR}/Source")

target_link_libraries(Engine
	PUBLIC
		glm::glm
    PRIVATE
        glfw
)
SetupGLFW(Engine)

target_link_libraries(Engine 
	PRIVATE 
		WinPixEventRuntime
)
target_compile_definitions(Engine 
	PRIVATE 
		$<$<CONFIG:Debug>:USE_PIX>
)

target_precompile_headers(Engine PUBLIC "pch.h")

if(WIN32)
    include(SetupWindows)
    SetupWin32(Engine)
	SetupD3D(Engine)
	target_link_libraries(Engine PUBLIC D3D12MemoryAllocator)
endif()