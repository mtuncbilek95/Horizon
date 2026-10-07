cmake_minimum_required(VERSION 3.26 FATAL_ERROR)

find_package(Python3 COMPONENTS Interpreter REQUIRED)

execute_process(
	COMMAND ${Python3_EXECUTABLE} "${CMAKE_SOURCE_DIR}/Tools/TypeLexer/GenerateReflection.py"
			--source "${CMAKE_CURRENT_SOURCE_DIR}"
			--root   "${CMAKE_SOURCE_DIR}/Source"
			--out    "${CMAKE_SOURCE_DIR}/Intermediate/TestLibrary"
	WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
	RESULT_VARIABLE REFLECT_GEN_RESULT
	OUTPUT_VARIABLE REFLECT_GEN_OUTPUT
	ERROR_VARIABLE  REFLECT_GEN_ERROR
)

if(NOT REFLECT_GEN_RESULT EQUAL 0)
	message(FATAL_ERROR "TestLibrary reflection generator failed:\n${REFLECT_GEN_ERROR}")
endif()

file(GLOB_RECURSE SUBDIRECTORIES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${SUBDIRECTORIES})

add_library(TestLibrary SHARED ${SUBDIRECTORIES})
HorizonModuleApi(TestLibrary TESTLIBRARY_API)

target_include_directories(TestLibrary PRIVATE "${CMAKE_SOURCE_DIR}/Intermediate/TestLibrary")
target_include_directories(TestLibrary PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")

target_link_libraries(TestLibrary
	PRIVATE
		Runtime Engine Editor
)

add_custom_command(TARGET TestLibrary POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_SOURCE_DIR}/ExampleProject/Plugins/TestLibrary"
	COMMAND ${CMAKE_COMMAND} -E copy_if_different
			$<TARGET_FILE:TestLibrary>
			"${CMAKE_SOURCE_DIR}/ExampleProject/Plugins/TestLibrary/TestLibrary.dll"
)