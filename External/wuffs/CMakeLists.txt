if(NOT DEFINED WUFFS_URL)
	set(WUFFS_URL https://github.com/google/wuffs-mirror-release-c.git)
endif()

FetchContent_Declare(
	wuffs
	EXCLUDE_FROM_ALL
	GIT_REPOSITORY ${WUFFS_URL}
	GIT_TAG v0.4.0-alpha.11
	GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(wuffs)

set(WUFFS_SOURCE "${wuffs_SOURCE_DIR}/release/c/wuffs-v0.4.c")
set_source_files_properties("${WUFFS_SOURCE}" PROPERTIES LANGUAGE CXX)

add_library(wuffs STATIC "${WUFFS_SOURCE}")
target_include_directories(wuffs PUBLIC "${wuffs_SOURCE_DIR}/release/c")
target_compile_definitions(wuffs PRIVATE WUFFS_IMPLEMENTATION)
target_compile_definitions(wuffs PUBLIC
	WUFFS_CONFIG__MODULES
	WUFFS_CONFIG__MODULE__BASE
	WUFFS_CONFIG__MODULE__AUX__BASE
	WUFFS_CONFIG__MODULE__AUX__IMAGE
	WUFFS_CONFIG__MODULE__ADLER32
	WUFFS_CONFIG__MODULE__CRC32
	WUFFS_CONFIG__MODULE__DEFLATE
	WUFFS_CONFIG__MODULE__ZLIB
	WUFFS_CONFIG__MODULE__PNG
	WUFFS_CONFIG__MODULE__JPEG
	WUFFS_CONFIG__MODULE__TARGA
	WUFFS_CONFIG__MODULE__BMP
)