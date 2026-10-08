cmake_minimum_required(VERSION 3.26 FATAL_ERROR)

# Always ask for snake3
find_package(Python3 COMPONENTS Interpreter REQUIRED)

# Defer the output from engine code
set(REFLECT_OUT_DIR "${CMAKE_SOURCE_DIR}/Intermediate/EditorApplication")

# Execute this shit.
execute_process(
	COMMAND ${Python3_EXECUTABLE} "${CMAKE_SOURCE_DIR}/Tools/TypeLexer/GenerateReflection.py"
			--source "${CMAKE_SOURCE_DIR}/Source/Runtime" "${CMAKE_SOURCE_DIR}/Source/Engine" "${CMAKE_SOURCE_DIR}/Source/Editor"
			--root "${CMAKE_SOURCE_DIR}/Source"
			--out "${REFLECT_OUT_DIR}"
	WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
	RESULT_VARIABLE REFLECT_GEN_RESULT
	OUTPUT_VARIABLE REFLECT_GEN_OUTPUT
	ERROR_VARIABLE REFLECT_GEN_ERROR
)

# Send the info
if(NOT REFLECT_GEN_RESULT EQUAL 0)
	message(FATAL_ERROR "Reflection generator failed for EditorApplication:\n${REFLECT_GEN_ERROR}")
endif()
message(STATUS "Reflection generated for EditorApplication:\n${REFLECT_GEN_OUTPUT}")

# Get the headers and sources and generate an executable
file(GLOB_RECURSE SUBDIRECTORIES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp" "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${SUBDIRECTORIES})

add_executable(EditorApplication ${SUBDIRECTORIES} "${REFLECT_OUT_DIR}/TypeManifestation.h")
source_group("Generated" FILES "${REFLECT_OUT_DIR}/TypeManifestation.h")

# Include the fuck out of it
target_include_directories(EditorApplication PRIVATE "${CMAKE_SOURCE_DIR}/Source")
target_include_directories(EditorApplication PRIVATE "${REFLECT_OUT_DIR}")

# Need those stuff in general
target_link_libraries(EditorApplication
	PUBLIC
		Runtime Engine Editor
)

# editor will be available only on windows so add this motherfucker here
set_target_properties(EditorApplication PROPERTIES WIN32_EXECUTABLE ON)
target_link_options(EditorApplication PRIVATE /ENTRY:mainCRTStartup)

# Copy the fuckout
add_custom_command(TARGET EditorApplication POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_if_different
			$<TARGET_RUNTIME_DLLS:EditorApplication>
			$<TARGET_FILE_DIR:EditorApplication>
	COMMAND_EXPAND_LISTS
)

# Copy the fuckout
add_custom_command(TARGET EditorApplication POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_if_different
			"${DXC_SIGNER_DLL}"
			$<TARGET_FILE_DIR:EditorApplication>
)