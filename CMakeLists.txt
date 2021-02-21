cmake_minimum_required(VERSION 3.8)

#Global cmake features
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

#Set startup project
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT TestFlightSimulator)

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

set(VULKAN_PATH $ENV{VULKAN_SDK})

add_library(vulkan INTERFACE)
target_link_libraries(vulkan INTERFACE ${VULKAN_PATH}/Lib/vulkan-1.lib)
target_include_directories(vulkan INTERFACE ${VULKAN_PATH}/Include)





file(GLOB_RECURSE SOURCES src/${TARGET}/private/*.cpp src/${TARGET}/public/*.cpp)
add_executable(TestFlightSimulator ${SOURCES})
	set_target_properties(TestFlightSimulator PROPERTIES 
		ARCHIVE_OUTPUT_DIRECTORY "${BINARIES_DIR}/Lib"
		RUNTIME_OUTPUT_DIRECTORY "${BINARIES_DIR}/Bin"
		LIBRARY_OUTPUT_DIRECTORY "${BINARIES_DIR}/Bin"
		VS_DEBUGGER_WORKING_DIRECTORY "${PROJECT_ROOT}")

target_link_libraries(TestFlightSimulator vulkan)
