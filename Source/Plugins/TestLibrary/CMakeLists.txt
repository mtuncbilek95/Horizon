cmake_minimum_required(VERSION 3.26 FATAL_ERROR)

# Always ask for snake3
find_package(Python3 COMPONENTS Interpreter REQUIRED)

# Defer the output from engine code
set(REFLECT_OUT_DIR "${CMAKE_SOURCE_DIR}/Intermediate/Plugins/")

# Execute this shit.
execute_process(
	COMMAND ${Python3_EXECUTABLE} "${CMAKE_SOURCE_DIR}/Tools/TypeLexer/GenerateReflection.py"
			--source "${CMAKE_CURRENT_SOURCE_DIR}"
			--root "${CMAKE_SOURCE_DIR}/Source/Plugins"
			--out "${REFLECT_OUT_DIR}"
	WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
	RESULT_VARIABLE REFLECT_GEN_RESULT
	OUTPUT_VARIABLE REFLECT_GEN_OUTPUT
	ERROR_VARIABLE REFLECT_GEN_ERROR
)

# Send the info
if(NOT REFLECT_GEN_RESULT EQUAL 0)
	message(FATAL_ERROR "Reflection generator failed for TestLibrary:\n${REFLECT_GEN_ERROR}")
endif()
message(STATUS "Reflection generated for TestLibrary:\n${REFLECT_GEN_OUTPUT}")

# Get the headers and sources and generate a shared library
file(GLOB_RECURSE SUBDIRECTORIES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp" "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${SUBDIRECTORIES})

add_library(TestLibrary ${HORIZON_MODULE_TYPE} ${SUBDIRECTORIES} "${REFLECT_OUT_DIR}/TypeManifestation.h")
source_group("Generated" FILES "${REFLECT_OUT_DIR}/TypeManifestation.h")

# Include the fuck out of it
target_include_directories(TestLibrary PRIVATE "${CMAKE_SOURCE_DIR}/Source/Plugins")
target_include_directories(TestLibrary PRIVATE "${REFLECT_OUT_DIR}")

# Check monolithic for Editor vs Standalone
if(HORIZON_MONOLITHIC)
	target_compile_definitions(TestLibrary PUBLIC TESTLIBRARY_API=)
else()
	target_compile_definitions(TestLibrary
		PRIVATE
			TESTLIBRARY_API=__declspec\(dllexport\)
		INTERFACE
			TESTLIBRARY_API=__declspec\(dllimport\)
	)
endif()

# Need those stuff in general
target_link_libraries(TestLibrary
	PRIVATE
		Runtime Engine
)

# If its monolithic, it means there is no editor
if(NOT HORIZON_MONOLITHIC)
	target_link_libraries(TestLibrary PRIVATE Editor)

	add_custom_command(TARGET TestLibrary POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_SOURCE_DIR}/ExampleProject/Plugins/TestLibrary"
		COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:TestLibrary> "${CMAKE_SOURCE_DIR}/ExampleProject/Plugins/TestLibrary/TestLibrary.dll"
	)
endif()