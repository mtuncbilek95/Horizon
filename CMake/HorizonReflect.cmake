function(HorizonReflect TARGET)
	cmake_parse_arguments(ARG "" "SOURCE_DIR;ROOT_DIR;OUT_DIR" "" ${ARGN})

	find_package(Python3 COMPONENTS Interpreter REQUIRED)

	set(REFLECT_GENERATOR "${HORIZON_ROOT}/Tools/TypeLexer/GenerateReflection.py")
	set(REFLECT_MANIFEST "${ARG_OUT_DIR}/TypeManifestation.h")

	file(GLOB_RECURSE REFLECT_HEADERS CONFIGURE_DEPENDS "${ARG_SOURCE_DIR}/*.h")

	add_custom_command(
		OUTPUT "${REFLECT_MANIFEST}"
		COMMAND "${Python3_EXECUTABLE}" "${REFLECT_GENERATOR}"
				--source "${ARG_SOURCE_DIR}"
				--root "${ARG_ROOT_DIR}"
				--out "${ARG_OUT_DIR}"
		DEPENDS ${REFLECT_HEADERS} "${REFLECT_GENERATOR}"
		WORKING_DIRECTORY "${ARG_ROOT_DIR}"
		COMMENT "Generating reflection for ${TARGET}"
		VERBATIM
	)

	target_sources(${TARGET} PRIVATE "${REFLECT_MANIFEST}")
	source_group("Generated" FILES "${REFLECT_MANIFEST}")
	target_include_directories(${TARGET} PRIVATE "${ARG_ROOT_DIR}" "${ARG_OUT_DIR}")
endfunction()