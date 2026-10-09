if(NOT DEFINED IMGUI_URL)
    set(IMGUI_URL https://github.com/ocornut/imgui.git)
    set(IMNODES_URL https://github.com/Nelarius/imnodes.git)
    set(IMGUIZMO_URL https://github.com/CedricGuillemet/ImGuizmo.git)
    set(IMGUI_NODE_EDITOR_URL https://github.com/thedmd/imgui-node-editor.git)
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
FetchContent_Declare(
    imguizmo
    EXCLUDE_FROM_ALL
    GIT_REPOSITORY ${IMGUIZMO_URL}
    GIT_TAG master
    GIT_SHALLOW TRUE
)
FetchContent_Declare(
    imgui_node_editor
    EXCLUDE_FROM_ALL
    GIT_REPOSITORY ${IMGUI_NODE_EDITOR_URL}
    GIT_TAG 021aa0ea4da13fed864bafb2a92d4c5205076866
    UPDATE_DISCONNECTED TRUE
    PATCH_COMMAND ${CMAKE_COMMAND} -DSOURCE_DIR=<SOURCE_DIR> -P ${CMAKE_SOURCE_DIR}/CMake/PatchImGuiNodeEditor.cmake
)
cmake_policy(SET CMP0169 OLD)
FetchContent_GetProperties(imnodes)
if(NOT imnodes_POPULATED)
    FetchContent_Populate(imnodes)
endif()
FetchContent_GetProperties(imguizmo)
if(NOT imguizmo_POPULATED)
    FetchContent_Populate(imguizmo)
endif()
FetchContent_GetProperties(imgui_node_editor)
if(NOT imgui_node_editor_POPULATED)
    FetchContent_Populate(imgui_node_editor)
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

file(GLOB TARGET_IMGUIZMO_FILES
    "${imguizmo_SOURCE_DIR}/src/ImGuizmo.h"
    "${imguizmo_SOURCE_DIR}/src/ImGuizmo.cpp"
)
source_group(TREE "${imguizmo_SOURCE_DIR}/src" FILES ${TARGET_IMGUIZMO_FILES})
add_library(ImGuizmo STATIC ${TARGET_IMGUIZMO_FILES})
target_include_directories(ImGuizmo PUBLIC "${imguizmo_SOURCE_DIR}/src")
target_link_libraries(ImGuizmo PUBLIC Imgui)

file(GLOB TARGET_NODE_EDITOR_FILES
    "${imgui_node_editor_SOURCE_DIR}/*.h"
    "${imgui_node_editor_SOURCE_DIR}/*.inl"
    "${imgui_node_editor_SOURCE_DIR}/*.cpp"
)
source_group(TREE "${imgui_node_editor_SOURCE_DIR}" FILES ${TARGET_NODE_EDITOR_FILES})
add_library(ImNodeEditor STATIC ${TARGET_NODE_EDITOR_FILES})
target_include_directories(ImNodeEditor PUBLIC "${imgui_node_editor_SOURCE_DIR}")
target_link_libraries(ImNodeEditor PUBLIC Imgui)

if(WIN32)
    include(ConfigureWindows)
    ConfigureD3D12Library(Imgui)
endif()

if(NOT HORIZON_MONOLITHIC)
	target_compile_definitions(Imgui PRIVATE IMGUI_API=__declspec\(dllexport\))
	target_compile_definitions(ImGuizmo PRIVATE IMGUI_API=__declspec\(dllexport\))
	target_compile_definitions(ImNodeEditor PRIVATE IMGUI_NODE_EDITOR_API=__declspec\(dllexport\))
endif()