cmake_minimum_required(VERSION 3.26 FATAL_ERROR)

find_package(Python3 COMPONENTS Interpreter REQUIRED)

execute_process(
    COMMAND ${Python3_EXECUTABLE} "${CMAKE_SOURCE_DIR}/Tools/TypeLexer/GenerateReflection.py"
            --source "${CMAKE_SOURCE_DIR}/Source"
            --out    "${CMAKE_SOURCE_DIR}/Intermediate"
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    RESULT_VARIABLE REFLECT_GEN_RESULT
    OUTPUT_VARIABLE REFLECT_GEN_OUTPUT
    ERROR_VARIABLE  REFLECT_GEN_ERROR
)

if(NOT REFLECT_GEN_RESULT EQUAL 0)
    message(FATAL_ERROR "Reflection generator failed:\n${REFLECT_GEN_ERROR}")
endif()

message(STATUS "Reflection generated:\n${REFLECT_GEN_OUTPUT}")

file(GLOB_RECURSE SUBDIRECTORIES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp" "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${SUBDIRECTORIES})

add_executable(EditorApplication ${SUBDIRECTORIES})

target_include_directories(EditorApplication PRIVATE "${CMAKE_SOURCE_DIR}/Source")
target_include_directories(EditorApplication PRIVATE "${CMAKE_SOURCE_DIR}/Intermediate")

target_link_libraries(EditorApplication
    PUBLIC
        Runtime Engine Editor
)

add_custom_command(TARGET EditorApplication POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_RUNTIME_DLLS:EditorApplication>
            $<TARGET_FILE_DIR:EditorApplication>
    COMMAND_EXPAND_LISTS
)

add_custom_command(TARGET EditorApplication POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${DXC_SIGNER_DLL}"
            $<TARGET_FILE_DIR:EditorApplication>
)