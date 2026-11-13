if(NOT DEFINED TINYDDSLOADER_URL)
	set(TINYDDSLOADER_URL https://github.com/benikabocha/tinyddsloader.git)
endif()

FetchContent_Declare(
	tinyddsloader
	EXCLUDE_FROM_ALL
	GIT_REPOSITORY ${TINYDDSLOADER_URL}
	GIT_TAG master
	SOURCE_SUBDIR none
)

FetchContent_MakeAvailable(tinyddsloader)

add_library(tinyddsloader INTERFACE)
target_include_directories(tinyddsloader INTERFACE "${tinyddsloader_SOURCE_DIR}")