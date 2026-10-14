include(ConfigureWindows)
include(HorizonReflect)

set(HORIZON_BUILD_DIR "${HORIZON_ROOT}/Build/${HORIZON_PRESET}")

if(NOT EXISTS "${HORIZON_BUILD_DIR}/CMakeCache.txt")
	message(FATAL_ERROR "Horizon is not configured with preset '${HORIZON_PRESET}', expected ${HORIZON_BUILD_DIR}")
endif()

load_cache("${HORIZON_BUILD_DIR}" READ_WITH_PREFIX HORIZON_ENGINE_
	CMAKE_BUILD_TYPE
	CMAKE_C_COMPILER
	CMAKE_CXX_COMPILER
	HORIZON_MONOLITHIC
)

if(HORIZON_ENGINE_HORIZON_MONOLITHIC)
	message(FATAL_ERROR "Horizon preset '${HORIZON_PRESET}' is monolithic, a game module needs the shared engine build")
endif()

if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE "${HORIZON_ENGINE_CMAKE_BUILD_TYPE}" CACHE STRING "Build type, follows the engine" FORCE)
elseif(NOT CMAKE_BUILD_TYPE STREQUAL HORIZON_ENGINE_CMAKE_BUILD_TYPE)
	message(FATAL_ERROR "Project is ${CMAKE_BUILD_TYPE} but Horizon '${HORIZON_PRESET}' is ${HORIZON_ENGINE_CMAKE_BUILD_TYPE}")
endif()

if(NOT CMAKE_C_COMPILER)
	set(CMAKE_C_COMPILER "${HORIZON_ENGINE_CMAKE_C_COMPILER}")
endif()

if(NOT CMAKE_CXX_COMPILER)
	set(CMAKE_CXX_COMPILER "${HORIZON_ENGINE_CMAKE_CXX_COMPILER}")
endif()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

function(HorizonImportModule NAME)
	set(MODULE_DLL "${HORIZON_BUILD_DIR}/Source/${NAME}/${NAME}.dll")
	set(MODULE_LIB "${HORIZON_BUILD_DIR}/Source/${NAME}/${NAME}.lib")

	if(NOT EXISTS "${MODULE_LIB}")
		message(FATAL_ERROR "${NAME}.lib not found in ${HORIZON_BUILD_DIR}, build Horizon '${HORIZON_PRESET}' first")
	endif()

	add_library(${NAME} SHARED IMPORTED GLOBAL)
	set_target_properties(${NAME} PROPERTIES
		IMPORTED_LOCATION "${MODULE_DLL}"
		IMPORTED_IMPLIB "${MODULE_LIB}"
	)
	target_include_directories(${NAME} INTERFACE "${HORIZON_ROOT}/Source")
endfunction()

function(HorizonImportEngine)
	if(TARGET Engine)
		return()
	endif()

	set(DEPS_DIR "${HORIZON_BUILD_DIR}/_deps")

	HorizonImportModule(Runtime)
	target_include_directories(Runtime INTERFACE
		"${DEPS_DIR}/nlohmann_json-src/include"
		"${DEPS_DIR}/imgui-src"
		"${DEPS_DIR}/imgui-src/backends"
		"${DEPS_DIR}/imguizmo-src/src"
		"${DEPS_DIR}/imgui_node_editor-src"
	)
	target_compile_definitions(Runtime INTERFACE
		RUNTIME_API=__declspec\(dllimport\)
		IMGUI_API=__declspec\(dllimport\)
		IMGUI_NODE_EDITOR_API=__declspec\(dllimport\)
	)

	HorizonImportModule(Engine)
	target_compile_definitions(Engine INTERFACE ENGINE_API=__declspec\(dllimport\))
	target_link_libraries(Engine INTERFACE Runtime)
endfunction()

function(HorizonConfigureModule TARGET)
	if(WIN32)
		ConfigureWindowsDefinitions()
	endif()

	target_compile_definitions(${TARGET}
		PRIVATE
			$<$<CONFIG:Debug>:HORIZON_DEBUG=1>
			$<$<CONFIG:Debug>:HORIZON_MEMORY_TRACKING=1>
			$<$<CONFIG:Release>:HORIZON_RELEASE=1>
			HORIZON_RESOURCE_DIR="${HORIZON_ROOT}/Resources"
	)
endfunction()

function(HorizonGameModule NAME)
	cmake_parse_arguments(ARG "" "SOURCE_DIR;INTERMEDIATE_DIR;OUTPUT_DIR" "" ${ARGN})

	if(NOT ARG_INTERMEDIATE_DIR)
		set(ARG_INTERMEDIATE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Intermediate")
	endif()

	HorizonImportEngine()

	file(GLOB_RECURSE MODULE_FILES CONFIGURE_DEPENDS "${ARG_SOURCE_DIR}/*.cpp" "${ARG_SOURCE_DIR}/*.hpp" "${ARG_SOURCE_DIR}/*.h")
	source_group(TREE "${ARG_SOURCE_DIR}" FILES ${MODULE_FILES})

	add_library(${NAME} SHARED ${MODULE_FILES})

	string(TOUPPER "${NAME}" MODULE_UPPER)
	target_compile_definitions(${NAME}
		PRIVATE
			${MODULE_UPPER}_API=__declspec\(dllexport\)
	)

	HorizonConfigureModule(${NAME})

	HorizonReflect(${NAME}
		SOURCE_DIR "${ARG_SOURCE_DIR}"
		ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}"
		OUT_DIR "${ARG_INTERMEDIATE_DIR}"
	)

	target_link_libraries(${NAME} PRIVATE Engine Runtime)
	target_link_options(${NAME} PRIVATE "/PDBALTPATH:%_PDB%")

	set_target_properties(${NAME} PROPERTIES
		RUNTIME_OUTPUT_DIRECTORY "${ARG_OUTPUT_DIR}/$<CONFIG>"
	)

	add_custom_command(TARGET ${NAME} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E touch "$<TARGET_FILE_DIR:${NAME}>/${NAME}.reload"
	)

	HorizonLaunchConfig(${NAME} "${ARG_OUTPUT_DIR}")
endfunction()

function(HorizonJsonPath INPUT OUTPUT)
	file(TO_NATIVE_PATH "${INPUT}" NATIVE_PATH)
	string(REPLACE "\\" "\\\\" NATIVE_PATH "${NATIVE_PATH}")
	set(${OUTPUT} "${NATIVE_PATH}" PARENT_SCOPE)
endfunction()

function(HorizonLaunchConfig NAME OUTPUT_DIR)
	set(LAUNCH_TEMPLATE "${CMAKE_CURRENT_SOURCE_DIR}/launch.vs.json.in")

	if(NOT EXISTS "${LAUNCH_TEMPLATE}")
		return()
	endif()

	set(EDITOR_DIR "${HORIZON_BUILD_DIR}/Source/EditorApplication")

	HorizonJsonPath("${OUTPUT_DIR}/${CMAKE_BUILD_TYPE}/${NAME}.dll" LAUNCH_OUTPUT)
	HorizonJsonPath("${EDITOR_DIR}" HORIZON_EDITOR_DIR)
	HorizonJsonPath("${EDITOR_DIR}/EditorApplication.exe" HORIZON_EDITOR_EXE)
	HorizonJsonPath("${CMAKE_CURRENT_SOURCE_DIR}/${NAME}.hproject" HORIZON_PROJECT_FILE)

	set(HORIZON_LAUNCH_TARGET "${NAME}.dll (${LAUNCH_OUTPUT})")

	configure_file("${LAUNCH_TEMPLATE}" "${CMAKE_CURRENT_SOURCE_DIR}/.vs/launch.vs.json" @ONLY)
endfunction()