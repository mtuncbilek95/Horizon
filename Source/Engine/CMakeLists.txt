cmake_minimum_required(VERSION 3.26 FATAL_ERROR)

file(GLOB_RECURSE SUBDIRECTORIES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp" "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${SUBDIRECTORIES})

add_library(Engine STATIC ${SUBDIRECTORIES})

target_include_directories(Engine PRIVATE "${CMAKE_SOURCE_DIR}/Source")
target_include_directories(Engine PUBLIC "${CMAKE_SOURCE_DIR}/Intermediate/Engine")

target_link_libraries(Engine
	PUBLIC
		Runtime
)

target_link_libraries(Engine
	PUBLIC
		stb
)

AddStbMacros(Engine)