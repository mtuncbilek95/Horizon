cmake_minimum_required(VERSION 3.26 FATAL_ERROR)

file(GLOB_RECURSE SUBDIRECTORIES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp" "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${SUBDIRECTORIES})

add_library(Runtime ${HORIZON_MODULE_TYPE} ${SUBDIRECTORIES})

target_include_directories(Runtime PUBLIC "${CMAKE_SOURCE_DIR}/Source")

if(HORIZON_MONOLITHIC)
	target_compile_definitions(Runtime
		PUBLIC
			RUNTIME_API=
			IMGUI_API=
			IMGUI_NODE_EDITOR_API=
	)
else()
	target_compile_definitions(Runtime
		PRIVATE
			RUNTIME_API=__declspec\(dllexport\)
			IMGUI_API=__declspec\(dllexport\)
			IMGUI_NODE_EDITOR_API=__declspec\(dllexport\)
		INTERFACE
			RUNTIME_API=__declspec\(dllimport\)
			IMGUI_API=__declspec\(dllimport\)
			IMGUI_NODE_EDITOR_API=__declspec\(dllimport\)
	)
endif()

target_link_libraries(Runtime
	PUBLIC
		nlohmann_json::nlohmann_json
	PRIVATE
		$<LINK_LIBRARY:WHOLE_ARCHIVE,Imgui,ImGuizmo,ImNodeEditor> DirectXShaderCompiler
)

target_include_directories(Runtime PUBLIC $<TARGET_PROPERTY:Imgui,INTERFACE_INCLUDE_DIRECTORIES>)
target_include_directories(Runtime PUBLIC $<TARGET_PROPERTY:ImGuizmo,INTERFACE_INCLUDE_DIRECTORIES>)
target_include_directories(Runtime PUBLIC $<TARGET_PROPERTY:ImNodeEditor,INTERFACE_INCLUDE_DIRECTORIES>)

if(WIN32)
	ConfigureWindowsLibrary(Runtime)
	ConfigureD3D12Library(Runtime)

	target_link_libraries(Runtime PRIVATE D3D12MemoryAllocator)
endif()