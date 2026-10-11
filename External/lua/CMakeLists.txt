if(NOT DEFINED LUA_URL)
	set(LUA_URL https://www.lua.org/ftp/lua-5.4.7.tar.gz)
endif()

FetchContent_Declare(
	lua
	EXCLUDE_FROM_ALL
	URL ${LUA_URL}
	URL_HASH SHA256=9fbf5e28ef86c69858f6d3d34eccc32e911c1a28b4120ff3e84aaa70cfbf1e30
	DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

FetchContent_MakeAvailable(lua)

file(GLOB LUA_SOURCES "${lua_SOURCE_DIR}/src/*.c")
list(REMOVE_ITEM LUA_SOURCES "${lua_SOURCE_DIR}/src/lua.c" "${lua_SOURCE_DIR}/src/luac.c")
set_source_files_properties(${LUA_SOURCES} PROPERTIES LANGUAGE CXX)

add_library(lua STATIC ${LUA_SOURCES})
target_include_directories(lua PUBLIC "${lua_SOURCE_DIR}/src")
target_compile_definitions(lua PRIVATE _CRT_SECURE_NO_WARNINGS)