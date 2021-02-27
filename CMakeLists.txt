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
message("vulkan sdk : " ${VULKAN_PATH})


file(GLOB_RECURSE SOURCES src/private/*.cpp src/public/*.h)
add_executable(TestFlightSimulator ${SOURCES})
	set_target_properties(TestFlightSimulator PROPERTIES 
		ARCHIVE_OUTPUT_DIRECTORY "${BINARIES_DIR}/Lib"
		RUNTIME_OUTPUT_DIRECTORY "${BINARIES_DIR}/Bin"
		LIBRARY_OUTPUT_DIRECTORY "${BINARIES_DIR}/Bin"
		VS_DEBUGGER_WORKING_DIRECTORY "${PROJECT_ROOT}")
target_include_directories(TestFlightSimulator PUBLIC ${PROJECT_ROOT}/src/public)
target_compile_features(TestFlightSimulator PUBLIC cxx_std_20)
set_target_properties(TestFlightSimulator PROPERTIES CXX_EXTENSIONS OFF)
source_group(TREE ${PROJECT_ROOT}/src FILES ${SOURCES})

IF (WIN32)
ELSE ()
	target_link_libraries(TestFlightSimulator -ldl -lpthread)
ENDIF()

# ASSIMP
target_include_directories(TestFlightSimulator PUBLIC ${PROJECT_ROOT}/third_party/assimp/include)
IF (WIN32)
	target_link_libraries(TestFlightSimulator ${PROJECT_ROOT}/temp/third_party/assimp-vc142-mt.lib)
ELSE()
	target_link_libraries(TestFlightSimulator ${PROJECT_ROOT}/temp/third_party/assimp-vc142-mt.a)
ENDIF()

# SPIRV-CROSS
target_include_directories(TestFlightSimulator PUBLIC ${PROJECT_ROOT}/third_party/SPIRV-cross)
IF (WIN32)
	target_link_libraries(TestFlightSimulator ${PROJECT_ROOT}/temp/third_party/spirv-cross-core.lib)
	target_link_libraries(TestFlightSimulator ${PROJECT_ROOT}/temp/third_party/spirv-cross-glsl.lib)
ELSE()
	target_link_libraries(TestFlightSimulator ${PROJECT_ROOT}/temp/third_party/spirv-cross-core.a)
	target_link_libraries(TestFlightSimulator ${PROJECT_ROOT}/temp/third_party/spirv-cross-glsl.a)
ENDIF()

# VMA
target_include_directories(TestFlightSimulator PUBLIC ${PROJECT_ROOT}/third_party/VulkanMemoryAllocator/src)

# VULKAN
target_include_directories(TestFlightSimulator PUBLIC ${VULKAN_PATH}/Include)
IF (WIN32)
	target_link_libraries(TestFlightSimulator ${VULKAN_PATH}/Lib/vulkan-1.lib)
ELSE()
	target_link_libraries(TestFlightSimulator -lvulkan)
ENDIF()

# GLFW
target_include_directories(TestFlightSimulator PUBLIC ${PROJECT_ROOT}/third_party/glfw/include)
IF (WIN32)
	target_link_libraries(TestFlightSimulator ${PROJECT_ROOT}/temp/third_party/glfw3.lib)
ELSE()
	target_link_libraries(TestFlightSimulator ${PROJECT_ROOT}/temp/third_party/libglfw3.a)
ENDIF()