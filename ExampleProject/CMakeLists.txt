cmake_minimum_required(VERSION 3.26 FATAL_ERROR)

set(HORIZON_ROOT "" CACHE PATH "Horizon engine root directory")
set(HORIZON_PRESET "Win64-Debug" CACHE STRING "Engine preset this project builds against")

if(NOT EXISTS "${HORIZON_ROOT}/CMake/HorizonProject.cmake")
	message(FATAL_ERROR "HORIZON_ROOT does not point to a Horizon engine: '${HORIZON_ROOT}'")
endif()

list(APPEND CMAKE_MODULE_PATH "${HORIZON_ROOT}/CMake")
include(HorizonProject)

project("ExampleProject" LANGUAGES C CXX)

HorizonGameModule(${PROJECT_NAME}
	SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Code"
	INTERMEDIATE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Intermediate"
	OUTPUT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Binaries"
)