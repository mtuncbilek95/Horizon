if(NOT DEFINED ufbx_URL)
    set(ufbx_URL https://github.com/ufbx/ufbx.git)
endif()

FetchContent_Declare(
    ufbx
    EXCLUDE_FROM_ALL
    GIT_REPOSITORY ${ufbx_URL}
    GIT_TAG v0.18.0
    GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(ufbx)

set(TARGET_ufbx_FILES
    "${ufbx_SOURCE_DIR}/ufbx.h"
    "${ufbx_SOURCE_DIR}/ufbx.c"
)
source_group(TREE "${ufbx_SOURCE_DIR}" FILES ${TARGET_ufbx_FILES})
add_library(ufbx STATIC ${TARGET_ufbx_FILES})
set_source_files_properties("${ufbx_SOURCE_DIR}/ufbx.c" PROPERTIES LANGUAGE CXX)
target_include_directories(ufbx PUBLIC ${ufbx_SOURCE_DIR})