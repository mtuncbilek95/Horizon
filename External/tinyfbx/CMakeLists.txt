if(NOT DEFINED UFBX_URL)
    set(UFBX_URL https://github.com/ufbx/ufbx.git)
endif()

FetchContent_Declare(
    ufbx
    EXCLUDE_FROM_ALL
    GIT_REPOSITORY ${UFBX_URL}
    GIT_TAG v0.18.0
    GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(ufbx)

set(TARGET_UFBX_FILES
    "${ufbx_SOURCE_DIR}/ufbx.h"
    "${ufbx_SOURCE_DIR}/ufbx.c"
)
source_group(TREE "${ufbx_SOURCE_DIR}" FILES ${TARGET_UFBX_FILES})
add_library(TinyFbx STATIC ${TARGET_UFBX_FILES})
set_source_files_properties("${ufbx_SOURCE_DIR}/ufbx.c" PROPERTIES LANGUAGE CXX)
target_include_directories(TinyFbx PUBLIC ${ufbx_SOURCE_DIR})