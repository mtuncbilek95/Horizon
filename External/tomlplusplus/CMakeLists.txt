cmake_minimum_required(VERSION 3.26 FATAL_ERROR)

include(FetchContent)

# Make URL variable
if(NOT DEFINED TOMLPLUSPLUS_URL)
    set(TOMLPLUSPLUS_URL https://github.com/marzer/tomlplusplus.git)
endif()

# toml++ Library
FetchContent_Declare(
        tomlplusplus
        EXCLUDE_FROM_ALL
        GIT_REPOSITORY ${TOMLPLUSPLUS_URL}
        GIT_TAG        v3.4.0
        GIT_SHALLOW    TRUE
)

FetchContent_MakeAvailable(tomlplusplus)
