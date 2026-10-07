cmake_minimum_required(VERSION 3.26 FATAL_ERROR)

HorizonModule(Editor EDITOR_API)

target_link_libraries(Editor
	PUBLIC
		Engine Runtime
	PRIVATE
		TinyFbx
)