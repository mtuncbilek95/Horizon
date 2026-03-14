cmake_minimum_required(VERSION 3.26 FATAL_ERROR)

include(FetchContent)

# Make URL variable
if(NOT DEFINED VOLK_URL)
    set(VOLK_URL https://github.com/zeux/volk.git)
endif()

# GLFW Library
FetchContent_Declare(
        volk
        EXCLUDE_FROM_ALL
        GIT_REPOSITORY ${VOLK_URL}
        GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(volk)