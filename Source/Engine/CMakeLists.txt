cmake_minimum_required(VERSION 3.20 FATAL_ERROR)

file(GLOB_RECURSE SUBDIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${SUBDIRECTORIES})

add_library(Engine STATIC ${SUBDIRECTORIES})

target_include_directories(Engine PRIVATE "${CMAKE_SOURCE_DIR}/Source")
target_link_libraries(Engine PUBLIC Runtime Imgui EnTT::EnTT assimp::assimp nlohmann_json)

include(SetupVulkan)
SetupVulkan(Engine)

install(TARGETS Engine
    ARCHIVE DESTINATION "lib"
)