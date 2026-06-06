cmake_minimum_required(VERSION 3.26 FATAL_ERROR)

include(FetchContent)

if(NOT DEFINED STB_URL)
    set(STB_URL https://github.com/nothings/stb.git)
endif()

# STBI Library
FetchContent_Declare(
        stb
        EXCLUDE_FROM_ALL
        GIT_REPOSITORY ${STB_URL}
        GIT_TAG master
        GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(stb)

add_library(stb INTERFACE)
target_include_directories(stb INTERFACE ${stb_SOURCE_DIR})

function(AddStbMacros TARGET)
	target_compile_definitions(${TARGET} PUBLIC STB_IMAGE_IMPLEMENTATION)
	target_compile_definitions(${TARGET} PUBLIC STB_IMAGE_WRITE_IMPLEMENTATION)
endfunction()