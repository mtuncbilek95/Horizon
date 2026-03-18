cmake_minimum_required(VERSION 3.26)

include(FetchContent)

# Make URL variable
if(NOT DEFINED JSON_URL)
    set(JSON_URL https://github.com/nlohmann/json.git)
endif()

# JSON Library
FetchContent_Declare(
        nlohmann
        EXCLUDE_FROM_ALL
        GIT_REPOSITORY ${JSON_URL}
        GIT_TAG v3.11.3
        GIT_SHALLOW TRUE
)

set(JSON_BuildTests OFF CACHE INTERNAL "")
set(nlohmann_json_BuildTests OFF CACHE INTERNAL "")
set(nlohmann_json_BUILD_TESTING OFF CACHE INTERNAL "")
set(nlohmann_json_BUILD_UNIT_TESTS OFF CACHE INTERNAL "")
set(nlohmann_json_ENABLE_TESTING OFF CACHE INTERNAL "")
set(nlohmann_json_BuildExamples OFF CACHE INTERNAL "")
set(nlohmann_json_BuildTests OFF CACHE INTERNAL "")

FetchContent_MakeAvailable(nlohmann)