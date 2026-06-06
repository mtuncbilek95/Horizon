include(FetchContent)

FetchContent_Declare(WinPixEventRuntimePkg
    URL https://www.nuget.org/api/v2/package/WinPixEventRuntime/1.0.240308001
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(WinPixEventRuntimePkg)

add_library(WinPixEventRuntime SHARED IMPORTED GLOBAL)
set_target_properties(WinPixEventRuntime PROPERTIES
    IMPORTED_LOCATION             "${winpixeventruntimepkg_SOURCE_DIR}/bin/x64/WinPixEventRuntime.dll"
    IMPORTED_IMPLIB               "${winpixeventruntimepkg_SOURCE_DIR}/bin/x64/WinPixEventRuntime.lib"
    INTERFACE_INCLUDE_DIRECTORIES "${winpixeventruntimepkg_SOURCE_DIR}/Include/WinPixEventRuntime"
)