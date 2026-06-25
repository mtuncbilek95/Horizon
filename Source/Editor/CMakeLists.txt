cmake_minimum_required(VERSION 3.26 FATAL_ERROR)

file(GLOB_RECURSE SUBDIRECTORIES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp" "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${SUBDIRECTORIES})

add_library(Editor STATIC ${SUBDIRECTORIES})

target_include_directories(Editor PRIVATE "${CMAKE_SOURCE_DIR}/Source")

target_link_libraries(Editor
	PUBLIC
		Engine Runtime Imgui
)



