cmake_minimum_required(VERSION 3.26 FATAL_ERROR)
include(FetchContent)

if(NOT DEFINED VMA_URL)
    set(VMA_URL https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git)
endif()

FetchContent_Declare(
    vma
    EXCLUDE_FROM_ALL
    GIT_REPOSITORY ${VMA_URL}
    GIT_TAG v3.2.1
    GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(vma)