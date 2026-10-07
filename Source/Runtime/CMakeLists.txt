cmake_minimum_required(VERSION 3.26 FATAL_ERROR)

HorizonModule(Runtime RUNTIME_API)
HorizonModuleApi(Runtime IMGUI_API)

target_link_libraries(Runtime
	PUBLIC
		nlohmann_json::nlohmann_json
	PRIVATE
		$<LINK_LIBRARY:WHOLE_ARCHIVE,Imgui,ImGuizmo> DirectXShaderCompiler
)

target_include_directories(Runtime PUBLIC $<TARGET_PROPERTY:Imgui,INTERFACE_INCLUDE_DIRECTORIES>)
target_include_directories(Runtime PUBLIC $<TARGET_PROPERTY:ImGuizmo,INTERFACE_INCLUDE_DIRECTORIES>)

if(WIN32)
	ConfigureWindowsLibrary(Runtime)
	ConfigureD3D12Library(Runtime)

	target_link_libraries(Runtime PRIVATE D3D12MemoryAllocator)
endif()