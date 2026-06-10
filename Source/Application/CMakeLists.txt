cmake_minimum_required(VERSION 3.26 FATAL_ERROR)

file(GLOB_RECURSE SUBDIRECTORIES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp" "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${SUBDIRECTORIES})

add_executable(Application ${SUBDIRECTORIES})

target_include_directories(Application PRIVATE "${CMAKE_SOURCE_DIR}/Source")

target_link_libraries(Application
    PUBLIC
        Engine Runtime
)

set_target_properties(Application PROPERTIES
    OUTPUT_NAME "CompactEngine"
)

include("${CMAKE_SOURCE_DIR}/CMake/CompileShadersDXC.cmake")
include("${CMAKE_SOURCE_DIR}/CMake/CookAsset.cmake")

set(SHADER_SRC_DIR "${CMAKE_SOURCE_DIR}/Shaders")
set(SHADER_OUT_DIR "${CMAKE_BINARY_DIR}/Shaders")
file(MAKE_DIRECTORY ${SHADER_OUT_DIR})

file(GLOB SHADER_SOURCES CONFIGURE_DEPENDS "${SHADER_SRC_DIR}/*.hlsl")

set(SHADER_OUTPUTS "")
foreach(SHADER_SRC ${SHADER_SOURCES})
    get_filename_component(SHADER_FILE ${SHADER_SRC} NAME)
    get_filename_component(OUT_NAME    ${SHADER_SRC} NAME_WLE)
    get_filename_component(STAGE_EXT   ${OUT_NAME}   LAST_EXT)

    if(STAGE_EXT STREQUAL ".vert")
        CompileShader("${SHADER_FILE}" "vs_6_6" "VSMain" "${OUT_NAME}")
    elseif(STAGE_EXT STREQUAL ".frag")
        CompileShader("${SHADER_FILE}" "ps_6_6" "PSMain" "${OUT_NAME}")
    elseif(STAGE_EXT STREQUAL ".comp")
        CompileShader("${SHADER_FILE}" "cs_6_6" "CSMain" "${OUT_NAME}")
    elseif(STAGE_EXT STREQUAL ".mesh")
        CompileShader("${SHADER_FILE}" "ms_6_6" "MSMain" "${OUT_NAME}")
    elseif(STAGE_EXT STREQUAL ".task")
        CompileShader("${SHADER_FILE}" "as_6_6" "ASMain" "${OUT_NAME}")
    else()
        message(WARNING "Bilinmeyen shader stage, atlandi: ${SHADER_FILE}")
    endif()
endforeach()

add_custom_target(Shaders ALL DEPENDS ${SHADER_OUTPUTS})
add_dependencies(Application Shaders)

#set(ASSET_OUT_DIR "${CMAKE_BINARY_DIR}/Cooked")
#SetupAssetCooker("${CMAKE_SOURCE_DIR}/Tools/AssetCooker" "${CMAKE_BINARY_DIR}/AssetCooker" COOKER_EXECUTABLE)
#CookAssets("${COOKER_EXECUTABLE}" "${CMAKE_SOURCE_DIR}/Resources" "${ASSET_OUT_DIR}")
#add_dependencies(Application CookAssets)

install(TARGETS Application
    RUNTIME DESTINATION "bin"
)

install(DIRECTORY "${SHADER_OUT_DIR}/"
    DESTINATION "bin/Shaders"
)

#install(DIRECTORY "${ASSET_OUT_DIR}/"
#    DESTINATION "bin/Cooked"
#)

add_custom_command(TARGET Application POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:WinPixEventRuntime>
        $<TARGET_FILE_DIR:Application>
)

install(FILES "$<TARGET_FILE:WinPixEventRuntime>" DESTINATION "bin")
