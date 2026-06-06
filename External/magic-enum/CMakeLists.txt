cmake_minimum_required(VERSION 3.26 FATAL_ERROR)

include(FetchContent)

# Make URL variable
if(NOT DEFINED MAGIC_URL)
    set(MAGIC_URL https://github.com/Neargye/magic_enum.git)
endif()

FetchContent_Declare(
        magic_enum
        EXCLUDE_FROM_ALL
        GIT_REPOSITORY ${MAGIC_URL}
        GIT_TAG v0.9.7
        GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(magic_enum)