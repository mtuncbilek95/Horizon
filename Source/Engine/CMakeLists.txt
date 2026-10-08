cmake_minimum_required(VERSION 3.26 FATAL_ERROR)

# Get the headers and sources and generate a shared library
file(GLOB_RECURSE SUBDIRECTORIES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp" "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${SUBDIRECTORIES})

add_library(Engine ${HORIZON_MODULE_TYPE} ${SUBDIRECTORIES})

# Include the fuck out of it
target_include_directories(Engine PUBLIC "${CMAKE_SOURCE_DIR}/Source")

# Check monolithic for Editor vs Standalone
if(HORIZON_MONOLITHIC)
	target_compile_definitions(Engine PUBLIC ENGINE_API=)
else()
	target_compile_definitions(Engine
		PRIVATE
			ENGINE_API=__declspec\(dllexport\)
		INTERFACE
			ENGINE_API=__declspec\(dllimport\)
	)
endif()

# Need those stuff in general
target_link_libraries(Engine
	PUBLIC
		Runtime
	PRIVATE
		Jolt
)