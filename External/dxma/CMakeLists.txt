cmake_minimum_required(VERSION 3.26 FATAL_ERROR)
include(FetchContent)

if(NOT DEFINED DXMA_URL)
    set(DXMA_URL https://github.com/GPUOpen-LibrariesAndSDKs/D3D12MemoryAllocator.git)
endif()


FetchContent_Declare(
    D3D12MemoryAllocator
    GIT_REPOSITORY ${DXMA_URL}
    GIT_TAG master
)

FetchContent_MakeAvailable(D3D12MemoryAllocator)