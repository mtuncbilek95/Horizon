get_filename_component(HORIZON_ROOT_PATH "${CMAKE_CURRENT_LIST_DIR}/.." ABSOLUTE)
set(HORIZON_ROOT "${HORIZON_ROOT_PATH}" CACHE INTERNAL "Horizon engine root")
set(HORIZON_GENERATED_DIR "${CMAKE_BINARY_DIR}/Generated" CACHE INTERNAL "Generated sources root")
set(HORIZON_REFLECTION_SCRIPT "${HORIZON_ROOT}/Tools/TypeLexer/GenerateReflection.py" CACHE INTERNAL "Reflection generator")

function(HorizonModuleApi TARGET API_MACRO)
	if(HORIZON_MONOLITHIC)
		target_compile_definitions(${TARGET} PUBLIC ${API_MACRO}=)
	else()
		target_compile_definitions(${TARGET}
			PRIVATE
				${API_MACRO}=__declspec\(dllexport\)
			INTERFACE
				${API_MACRO}=__declspec\(dllimport\)
		)
	endif()
endfunction()

function(HorizonModule NAME API_MACRO)
	file(GLOB_RECURSE MODULE_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp" "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
	source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${MODULE_SOURCES})

	add_library(${NAME} ${HORIZON_MODULE_TYPE} ${MODULE_SOURCES})

	target_include_directories(${NAME} PUBLIC "${HORIZON_ROOT}/Source")

	HorizonModuleApi(${NAME} ${API_MACRO})
endfunction()

function(HorizonReflection TARGET INCLUDE_ROOT)
	find_package(Python3 COMPONENTS Interpreter REQUIRED)

	set(REFLECT_OUT_DIR "${HORIZON_GENERATED_DIR}/${TARGET}")
	set(REFLECT_OUTPUT "${REFLECT_OUT_DIR}/TypeManifestation.h")
	set(REFLECT_HEADERS "")

	foreach(SOURCE_DIR ${ARGN})
		file(GLOB_RECURSE DIR_HEADERS CONFIGURE_DEPENDS "${SOURCE_DIR}/*.h")
		list(APPEND REFLECT_HEADERS ${DIR_HEADERS})
	endforeach()

	add_custom_command(
		OUTPUT "${REFLECT_OUTPUT}"
		COMMAND ${Python3_EXECUTABLE} "${HORIZON_REFLECTION_SCRIPT}"
				--source ${ARGN}
				--root "${INCLUDE_ROOT}"
				--out "${REFLECT_OUT_DIR}"
		DEPENDS ${REFLECT_HEADERS} "${HORIZON_REFLECTION_SCRIPT}"
		WORKING_DIRECTORY "${HORIZON_ROOT}"
		COMMENT "Generating reflection for ${TARGET}"
		VERBATIM
	)

	target_sources(${TARGET} PRIVATE "${REFLECT_OUTPUT}")
	target_include_directories(${TARGET} PRIVATE "${REFLECT_OUT_DIR}")
	source_group("Generated" FILES "${REFLECT_OUTPUT}")
endfunction()

function(HorizonPlugin NAME SOURCE_DIR)
	cmake_parse_arguments(PLUGIN "" "OUTPUT_DIR" "" ${ARGN})

	file(GLOB_RECURSE PLUGIN_SOURCES CONFIGURE_DEPENDS "${SOURCE_DIR}/*.cpp" "${SOURCE_DIR}/*.hpp" "${SOURCE_DIR}/*.h")
	source_group(TREE "${SOURCE_DIR}" FILES ${PLUGIN_SOURCES})

	add_library(${NAME} ${HORIZON_MODULE_TYPE} ${PLUGIN_SOURCES})

	string(TOUPPER "${NAME}" PLUGIN_NAME_UPPER)
	HorizonModuleApi(${NAME} ${PLUGIN_NAME_UPPER}_API)

	get_filename_component(PLUGIN_INCLUDE_ROOT "${SOURCE_DIR}" DIRECTORY)
	target_include_directories(${NAME} PRIVATE "${PLUGIN_INCLUDE_ROOT}")

	target_link_libraries(${NAME} PRIVATE Runtime Engine)

	if(NOT HORIZON_MONOLITHIC)
		target_link_libraries(${NAME} PRIVATE Editor)
	endif()

	HorizonReflection(${NAME} "${PLUGIN_INCLUDE_ROOT}" "${SOURCE_DIR}")

	if(PLUGIN_OUTPUT_DIR AND NOT HORIZON_MONOLITHIC)
		add_custom_command(TARGET ${NAME} POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E make_directory "${PLUGIN_OUTPUT_DIR}"
			COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:${NAME}> "${PLUGIN_OUTPUT_DIR}/${NAME}.dll"
		)
	endif()
endfunction()
