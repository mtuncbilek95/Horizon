cmake_minimum_required(VERSION 3.26 FATAL_ERROR)

file(GLOB_RECURSE SUBDIRECTORIES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp" "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${SUBDIRECTORIES})

add_executable(EditorApplication ${SUBDIRECTORIES})

target_include_directories(EditorApplication PRIVATE "${HORIZON_ROOT}/Source")

target_link_libraries(EditorApplication
	PUBLIC
		Runtime Engine Editor
)

HorizonReflection(EditorApplication "${HORIZON_ROOT}/Source"
	"${HORIZON_ROOT}/Source/Runtime"
	"${HORIZON_ROOT}/Source/Engine"
	"${HORIZON_ROOT}/Source/Editor"
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
