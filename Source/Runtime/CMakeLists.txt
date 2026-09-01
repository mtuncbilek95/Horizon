cmake_minimum_required(VERSION 3.26 FATAL_ERROR)

file(GLOB_RECURSE SUBDIRECTORIES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp" "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${SUBDIRECTORIES})

add_library(Runtime STATIC ${SUBDIRECTORIES})

target_include_directories(Runtime PRIVATE "${CMAKE_SOURCE_DIR}/Source")

target_link_libraries(Runtime
	PUBLIC
		nlohmann_json::nlohmann_json Imgui
)

target_link_libraries(Runtime 
	PUBLIC 
		DirectXShaderCompiler
)

if(WIN32)
    ConfigureWindowsLibrary(Runtime)
	ConfigureD3D12Library(Runtime)

	target_link_libraries(Runtime PUBLIC D3D12MemoryAllocator)
endif()