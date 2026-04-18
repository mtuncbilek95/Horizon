cmake_minimum_required(VERSION 3.20 FATAL_ERROR)

file(GLOB_RECURSE SUBDIRECTORIES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${SUBDIRECTORIES})

add_executable(Application ${SUBDIRECTORIES})

target_include_directories(Application PRIVATE "${CMAKE_SOURCE_DIR}/Source")
target_link_libraries(Application PRIVATE Runtime)

set_target_properties(Application PROPERTIES
    OUTPUT_NAME "Horizon - Game"
)

install(TARGETS Application
    RUNTIME DESTINATION "bin"
)