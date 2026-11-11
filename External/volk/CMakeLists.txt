if(NOT DEFINED VOLK_URL)
    set(VOLK_URL https://github.com/zeux/volk.git)
endif()

FetchContent_Declare(
        volk
        EXCLUDE_FROM_ALL
        GIT_REPOSITORY ${VOLK_URL}
        GIT_SHALLOW TRUE
)

if(WIN32)
    set(VOLK_STATIC_DEFINES VK_USE_PLATFORM_WIN32_KHR CACHE STRING "Volk platform defines" FORCE)
endif()

FetchContent_MakeAvailable(volk)