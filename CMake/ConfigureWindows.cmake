function(ConfigureWindowsDefinitions)
	add_compile_definitions(HORIZON_WINDOWS)
	add_compile_definitions(H_MANIFEST_EXPORT=__declspec\(dllexport\))
endfunction()

function(ConfigureWindowsLibrary TARGET)
	target_link_libraries(${TARGET} 
		PUBLIC 
			user32.lib gdi32.lib shell32.lib 
			Setupapi.lib windowscodecs.lib 
			Synchronization.lib dwmapi.lib 
			Shcore.lib GameInput.lib
	)
endfunction()

function(ConfigureD3D12Library TARGET)
	target_link_libraries(${TARGET} 
		PUBLIC 
			d3d11.lib d3d12.lib dxgi.lib 
			dxguid.lib d3dcompiler.lib
	)
endfunction()