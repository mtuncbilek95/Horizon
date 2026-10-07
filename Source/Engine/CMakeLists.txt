cmake_minimum_required(VERSION 3.26 FATAL_ERROR)

HorizonModule(Engine ENGINE_API)

target_link_libraries(Engine
	PUBLIC
		Runtime
	PRIVATE
		Jolt
)