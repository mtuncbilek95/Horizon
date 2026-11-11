function(ConfigureVulkan TARGET)
    find_package(Vulkan REQUIRED)

    if(Vulkan_FOUND)
		target_link_libraries(${TARGET}
			PRIVATE 
				volk
				d3dcompiler.lib
		)

		target_compile_definitions(${TARGET}
			PRIVATE 
				VMA_VULKAN_VERSION=1004000
				VK_NO_PROTOTYPES=1
		)

		target_include_directories(${TARGET} 
			PRIVATE
				$ENV{VULKAN_SDK}/Include
		)
    else()
        message(FATAL_ERROR "Could not find Vulkan")
	endif()
endfunction()