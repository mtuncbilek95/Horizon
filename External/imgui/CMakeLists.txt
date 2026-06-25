if(NOT DEFINED IMGUI_URL)
    set(IMGUI_URL https://github.com/ocornut/imgui.git)
    set(IMNODES_URL https://github.com/Nelarius/imnodes.git)
endif()

FetchContent_Declare(
    imgui
    EXCLUDE_FROM_ALL
    GIT_REPOSITORY ${IMGUI_URL}
    GIT_TAG docking
    GIT_SHALLOW TRUE
    OVERRIDE_FIND_PACKAGE
)
FetchContent_MakeAvailable(imgui)

FetchContent_Declare(
    imnodes
    EXCLUDE_FROM_ALL
    GIT_REPOSITORY ${IMNODES_URL}
    GIT_TAG master
    GIT_SHALLOW TRUE
)
cmake_policy(SET CMP0169 OLD)
FetchContent_GetProperties(imnodes)
if(NOT imnodes_POPULATED)
    FetchContent_Populate(imnodes)
endif()

file(GLOB IMGUI_COMMON_FILES
    "${imgui_SOURCE_DIR}/*.cpp"
    "${imgui_SOURCE_DIR}/*.h"
    "${imgui_SOURCE_DIR}/misc/cpp/imgui_stdlib.h"
    "${imgui_SOURCE_DIR}/misc/cpp/imgui_stdlib.cpp"
)

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(IMGUI_BACKEND_FILES
        "${imgui_SOURCE_DIR}/backends/imgui_impl_dx12.h"
        "${imgui_SOURCE_DIR}/backends/imgui_impl_dx12.cpp"
        "${imgui_SOURCE_DIR}/backends/imgui_impl_win32.h"
        "${imgui_SOURCE_DIR}/backends/imgui_impl_win32.cpp"
    )
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(IMGUI_BACKEND_FILES
        "${imgui_SOURCE_DIR}/backends/imgui_impl_osx.h"
        "${imgui_SOURCE_DIR}/backends/imgui_impl_osx.mm"
        "${imgui_SOURCE_DIR}/backends/imgui_impl_metal.h"
        "${imgui_SOURCE_DIR}/backends/imgui_impl_metal.mm"
    )
else()
    message(FATAL_ERROR "[CMAKE ERROR] -- Unsupported platform: ${CMAKE_SYSTEM_NAME}")
endif()

set(TARGET_SOURCE_FILES ${IMGUI_COMMON_FILES} ${IMGUI_BACKEND_FILES})
source_group(TREE "${imgui_SOURCE_DIR}" FILES ${TARGET_SOURCE_FILES})
add_library(Imgui STATIC ${TARGET_SOURCE_FILES})
target_include_directories(Imgui PUBLIC
    "${imgui_SOURCE_DIR}"
    "${imgui_SOURCE_DIR}/backends"
)

file(GLOB TARGET_IMNODE_FILES
    "${imnodes_SOURCE_DIR}/imnodes.h"
    "${imnodes_SOURCE_DIR}/imnodes.cpp"
    "${imnodes_SOURCE_DIR}/imnodes_internal.h"
)
source_group(TREE "${imnodes_SOURCE_DIR}" FILES ${TARGET_IMNODE_FILES})
add_library(ImNodes STATIC ${TARGET_IMNODE_FILES})
target_include_directories(ImNodes PUBLIC ${imnodes_SOURCE_DIR})
target_link_libraries(ImNodes PUBLIC Imgui)

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    include(SetupWindows)
        SetupD3D(Imgui)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    enable_language(OBJCXX)
    target_link_libraries(Imgui PUBLIC
        "-framework Metal"
        "-framework MetalKit"
        "-framework AppKit"
        "-framework QuartzCore"
    )
endif()
