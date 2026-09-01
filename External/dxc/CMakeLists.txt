cmake_minimum_required(VERSION 3.26 FATAL_ERROR)
include(FetchContent)

if(NOT DEFINED DXC_VERSION)
    set(DXC_VERSION 1.8.2502.8)
endif()

FetchContent_Declare(DirectXShaderCompilerPkg
    URL https://www.nuget.org/api/v2/package/Microsoft.Direct3D.DXC/${DXC_VERSION}
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(DirectXShaderCompilerPkg)

set(DXC_BIN_DIR     "${directxshadercompilerpkg_SOURCE_DIR}/build/native/bin/x64")
set(DXC_LIB_DIR     "${directxshadercompilerpkg_SOURCE_DIR}/build/native/lib/x64")
set(DXC_INCLUDE_DIR "${directxshadercompilerpkg_SOURCE_DIR}/build/native/include")

add_library(DirectXShaderCompiler SHARED IMPORTED GLOBAL)
set_target_properties(DirectXShaderCompiler PROPERTIES
    IMPORTED_LOCATION             "${DXC_BIN_DIR}/dxcompiler.dll"
    IMPORTED_IMPLIB               "${DXC_LIB_DIR}/dxcompiler.lib"
    INTERFACE_INCLUDE_DIRECTORIES "${DXC_INCLUDE_DIR}"
)

set(DXC_SIGNER_DLL "${DXC_BIN_DIR}/dxil.dll" CACHE INTERNAL "DXIL signing library")