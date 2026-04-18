cmake_minimum_required(VERSION 3.26 FATAL_ERROR)

# ----------------------- PROJECT NAME -----------------------
project("Horizon")
# ------------------------------------------------------------

# ----------------------- CXX FLAGS --------------------------
if(POLICY CMP0141)
	cmake_policy(SET CMP0141 NEW)
endif()

if(POLICY CMP0077)
	cmake_policy(SET CMP0077 NEW)
endif()

if(POLICY CMP0175)
	cmake_policy(SET CMP0175 NEW)
endif()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

set(FETCHCONTENT_UPDATES_DISCONNECTED ON CACHE BOOL "Enable Update Fetch" FORCE)
set(CMAKE_WARN_DEPRECATED OFF CACHE BOOL "CMake Warn Deprecated" FORCE)
set(CMAKE_SUPPRESS_DEVELOPER_WARNINGS OFF CACHE BOOL "CMake Suppresse Developer Warnings" FORCE)
# ------------------------------------------------------------

# ----------------- PREPROCESSOR DEFINITIONS -----------------
set(CMAKE_ROOT_DIR ${CMAKE_SOURCE_DIR})
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/CMake")
# ------------------------------------------------------------
 
# --------------------- SETUP CONFIG -------------------------
include(SetupConfig)
SetupConfig()

option(ENABLE_CATCH_TEST "Enable testing" ON)

if(ENABLE_CATCH_TEST)
	enable_testing()
endif()
# ------------------------------------------------------------

# ----------------------- SUBDIRECTORIES ---------------------
add_subdirectory("External")
add_subdirectory("Source/Runtime")
add_subdirectory("Source/Application")
# ------------------------------------------------------------