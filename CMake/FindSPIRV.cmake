function(BindSpirv TARGET)
	if(UNIX)
	    find_package(PkgConfig REQUIRED)
	    pkg_check_modules(SPIRV REQUIRED SPIRV-Tools)
	
	    target_link_libraries(${TARGET} PUBLIC ${SPIRV_LIBRARIES} 
	            -lglslang
	            -lSPIRV
	            -lSPIRV-Tools
	            -lSPIRV-Tools-opt
	            -lSPIRV-Tools-link
	            -lshaderc_shared)
	    target_include_directories(${TARGET} PRIVATE ${SPIRV_INCLUDE_DIRS})
	
	    pkg_check_modules(SHADERC shaderc)
	    if(SHADERC_FOUND)
	        target_link_libraries(${TARGET} PRIVATE ${SHADERC_LIBRARIES})
	        target_include_directories(${TARGET} PRIVATE ${SHADERC_INCLUDE_DIRS})
	    endif()
	endif()
endfunction()