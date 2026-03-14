cmake_minimum_required(VERSION 3.20 FATAL_ERROR)

file(GLOB_RECURSE DEPENDENCY "Source/*.cpp" "Source/*.h")
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/Source" PREFIX "Source" FILES ${DEPENDENCY})

add_library(Game STATIC ${DEPENDENCY})

target_include_directories(Game PUBLIC "${CMAKE_SOURCE_DIR}/Source")
target_link_libraries(Game PUBLIC Runtime Engine)

target_compile_definitions(Game PUBLIC
    GAME_ASSETS_DIR="${CMAKE_CURRENT_SOURCE_DIR}/Assets"
    GAME_SHADERS_DIR="${CMAKE_CURRENT_SOURCE_DIR}/Shaders"
)

install(TARGETS Game
    ARCHIVE DESTINATION "lib"
)

install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/Assets/"
    DESTINATION "assets"
)

install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/Shaders/"
    DESTINATION "shaders"
)