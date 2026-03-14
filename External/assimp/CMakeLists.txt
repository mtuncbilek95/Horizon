cmake_minimum_required(VERSION 3.26)

include(FetchContent)

if(POLICY CMP0175)
  cmake_policy(SET CMP0175 NEW)
endif()

# Make URL variable
if(NOT DEFINED ASSIMP_URL)
	set(ASSIMP_URL https://github.com/assimp/assimp.git)
endif()

# ASSIMP Library
FetchContent_Declare(
	assimp
    EXCLUDE_FROM_ALL
	GIT_REPOSITORY ${ASSIMP_URL}
	GIT_TAG v5.4.3
	GIT_SHALLOW TRUE
	QUIET
)

set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build shared libraries" FORCE)
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "Assimp build tests" FORCE)
set(ASSIMP_INSTALL OFF CACHE BOOL "Assimp install" FORCE)
set(ASSIMP_INSTALL_PDB OFF CACHE BOOL "Assimp install PDB" FORCE)
set(ASSIMP_WARNINGS_AS_ERRORS OFF CACHE BOOL "Assimp warnings as errors" FORCE)
set(ASSIMP_BUILD_SAMPLES OFF CACHE BOOL "" FORCE)
set(ASSIMP_INJECT_DEBUG_POSTFIX OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(assimp)