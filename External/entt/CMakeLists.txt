cmake_minimum_required(VERSION 3.26 FATAL_ERROR)

include(FetchContent)

# Make URL variable
if(NOT DEFINED ENTT_URL)
    set(ENTT_URL https://github.com/skypjack/entt.git)
endif()

# EnTT Library
FetchContent_Declare(
        EnTT
        EXCLUDE_FROM_ALL
        GIT_REPOSITORY ${ENTT_URL}
        GIT_TAG        v3.14.0
        GIT_SHALLOW    TRUE
)

set(ENTT_BUILD_TESTING OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(EnTT)
