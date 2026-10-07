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

	target_include_directories(${NAME} PUBLIC "${CMAKE_SOURCE_DIR}/Source")

	HorizonModuleApi(${NAME} ${API_MACRO})
endfunction()
