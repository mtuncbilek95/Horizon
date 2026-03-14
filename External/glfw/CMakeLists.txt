cmake_minimum_required(VERSION 3.26 FATAL_ERROR)

include(FetchContent)

# Make URL variable
if(NOT DEFINED GLFW_URL)
    set(GLFW_URL https://github.com/glfw/glfw.git)
endif()

# GLFW Library
FetchContent_Declare(
        glfw
        EXCLUDE_FROM_ALL
        GIT_REPOSITORY ${GLFW_URL}
        GIT_TAG 3.3.9
        GIT_SHALLOW TRUE
)

set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(glfw)