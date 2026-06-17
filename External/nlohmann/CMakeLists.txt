if(NOT DEFINED NLOHMANN_JSON_URL)
	set(NLOHMANN_JSON_URL https://github.com/nlohmann/json.git)
endif()

FetchContent_Declare(
	nlohmann_json
	EXCLUDE_FROM_ALL
	GIT_REPOSITORY ${NLOHMANN_JSON_URL}
	GIT_SHALLOW TRUE
)

set(JSON_BuildTests OFF CACHE BOOL "Disable nlohmann/json tests" FORCE)
set(JSON_Install OFF CACHE BOOL "Disable install targets" FORCE)

FetchContent_MakeAvailable(nlohmann_json)