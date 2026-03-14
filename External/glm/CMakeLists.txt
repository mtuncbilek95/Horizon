cmake_minimum_required(VERSION 3.26 FATAL_ERROR)

include(FetchContent)

# Make URL variable
if(NOT DEFINED GLM_URL)
	set(GLM_URL https://github.com/g-truc/glm.git)
endif()

# GLFW Library
FetchContent_Declare(
        glm
		EXCLUDE_FROM_ALL
		GIT_REPOSITORY	${GLM_URL}
		GIT_TAG bf71a834948186f4097caa076cd2663c69a10e1e #refs/tags/1.0.1
        GIT_SHALLOW TRUE
)

set(GLM_TEST_ENABLE OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glm)

function(DefineGlmMacros TARGET)
	target_compile_definitions(${TARGET} PUBLIC GLM_FORCE_SILENT_WARNINGS)
    target_compile_definitions(${TARGET} PUBLIC GLM_ENABLE_EXPERIMENTAL)
    target_compile_definitions(${TARGET} PUBLIC GLM_FORCE_ALIGNED_GENTYPES)
endfunction()