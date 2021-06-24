cmake_minimum_required(VERSION 3.13)
cmake_policy(SET CMP0077 NEW)

include(src/BuildFunctions.cmake)

#Global cmake features
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

#Set startup project
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT TestFlightSimulator)

# update submodules
message("-- updating git submodules ... ")
#execute_process(COMMAND git submodule update --init --recursive)

#set project name
project(TestFlightSimulator)

# Pure3D only support x64 architecture
if(CMAKE_SIZEOF_VOID_P EQUAL 4)
	message(FATAL_ERROR "32-bits is not supported")
endif()

# Set compiler options
if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
	# MP build for MSVC
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP /EHsc")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	# Suppress invalid offset warning on GCC
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-invalid-offsetof")
endif()

# Set project constants
set(PROJECT_ROOT ${CMAKE_CURRENT_SOURCE_DIR}) # Project dir
set(BINARIES_DIR ${PROJECT_ROOT}/bin) # Binaries dir
set(INTERMEDIATES_DIR ${PROJECT_ROOT}/temp) # Intermediates dir

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${BINARIES_DIR}/Lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${BINARIES_DIR}/Bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${BINARIES_DIR}/Bin")
set(CMAKE_VS_DEBUGGER_WORKING_DIRECTORY "${PROJECT_ROOT}")

set(FETCHCONTENT_UPDATES_DISCONNECTED ON)
	
add_subdirectory(third_party)
#add_subdirectory(src)