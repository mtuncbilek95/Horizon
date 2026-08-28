function(ConfigureWindowsDefinitions)
	add_compile_definitions(HORIZON_WINDOWS)

	add_compile_definitions(H_EXPORT=__declspec\(dllexport\))

	add_compile_definitions(FORCEINLINE=__forceinline)
	add_compile_definitions(NOINLINE=__declspec\(noinline\))
	add_compile_definitions(INLINE=__inline)
	add_compile_definitions(NULLPTR=decltype\(nullptr\)\(\))
	add_compile_definitions(RESTRICT=__restrict)
	add_compile_definitions(NODISCARD=\[\[nodiscard\]\])
endfunction()

function(ConfigureWindowsLibrary TARGET)
	target_link_libraries(${TARGET} 
		PUBLIC 
			user32.lib gdi32.lib shell32.lib 
			Setupapi.lib windowscodecs.lib 
			Synchronization.lib
	)
endfunction()

function(ConfigureD3D12Library TARGET)
	target_link_libraries(${TARGET} 
		PUBLIC 
			d3d11.lib d3d12.lib dxgi.lib 
			dxguid.lib d3dcompiler.lib
	)
endfunction()