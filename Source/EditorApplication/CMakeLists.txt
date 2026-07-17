cmake_minimum_required(VERSION 3.26 FATAL_ERROR)

file(GLOB_RECURSE SUBDIRECTORIES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp" "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${SUBDIRECTORIES})

add_executable(EditorApplication ${SUBDIRECTORIES})

target_include_directories(EditorApplication PRIVATE "${CMAKE_SOURCE_DIR}/Source")

target_link_libraries(EditorApplication
    PUBLIC
        Engine Runtime Editor
)