function(SetupVulkan TARGET)
    find_package(Vulkan REQUIRED)

    target_link_libraries(${TARGET} PRIVATE Vulkan::Headers volk::volk GPUOpen::VulkanMemoryAllocator)

    find_package(Vulkan OPTIONAL_COMPONENTS shaderc_combined)
    if(Vulkan_shaderc_combined_FOUND)
        target_link_libraries(${TARGET} PRIVATE Vulkan::shaderc_combined)
    endif()

    target_include_directories(${TARGET} PUBLIC ${Vulkan_INCLUDE_DIR})

    target_compile_definitions(${TARGET} PUBLIC VK_NO_PROTOTYPES)

    if(WIN32)
        target_compile_definitions(${TARGET} PUBLIC VK_USE_PLATFORM_WIN32_KHR)
    elseif(UNIX AND NOT APPLE)
        if(DEFINED ENV{WAYLAND_DISPLAY})
            target_compile_definitions(${TARGET} PUBLIC VK_USE_PLATFORM_WAYLAND_KHR)
        else()
            target_compile_definitions(${TARGET} PUBLIC VK_USE_PLATFORM_XLIB_KHR)
        endif()
    endif()

endfunction()