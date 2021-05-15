cmake_minimum_required(VERSION 3.8)

#Global cmake features
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

#Set startup project
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT TestFlightSimulator)

# update submodules
message("-- updating git submodules ... ")
execute_process(COMMAND git submodule update --init --recursive)

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

function(configure_project PROJECT_NAME SOURCES)
	set_target_properties(${PROJECT_NAME} PROPERTIES 
		ARCHIVE_OUTPUT_DIRECTORY "${BINARIES_DIR}/Lib"
		RUNTIME_OUTPUT_DIRECTORY "${BINARIES_DIR}/Bin"
		LIBRARY_OUTPUT_DIRECTORY "${BINARIES_DIR}/Bin"
		VS_DEBUGGER_WORKING_DIRECTORY "${PROJECT_ROOT}")
	target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)
	set_target_properties(${PROJECT_NAME} PROPERTIES CXX_EXTENSIONS OFF)
endfunction()

function(add_subfodler_target TARGET_NAME CONTAINER_FOLDER)
	add_subdirectory(${TARGET_NAME})
	_add_subfodler_target(${TARGET_NAME} ${TARGET_NAME} ${CONTAINER_FOLDER})
endfunction()

function(_add_subfodler_target CURRENT_DIRECTORY TARGET_NAME CONTAINER_FOLDER)
	# find recursively targets in subdirectories
    get_property(_subdirs DIRECTORY ${CURRENT_DIRECTORY} PROPERTY SUBDIRECTORIES)
    foreach(_subdir IN LISTS _subdirs)
        _add_subfodler_target(${_subdir} ${TARGET_NAME} ${CONTAINER_FOLDER})
    endforeach()
	
	# convert eventual relative paths to absolute paths
	if(NOT IS_ABSOLUTE ${CURRENT_DIRECTORY})
		set(CURRENT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${CURRENT_DIRECTORY})
	endif()
	
	# iterate over each target of the current directory	
    get_directory_property(_sub_targets DIRECTORY ${CURRENT_DIRECTORY} BUILDSYSTEM_TARGETS)
	foreach(_target ${_sub_targets})		
		file(RELATIVE_PATH _relative_path ${CMAKE_CURRENT_SOURCE_DIR} ${CURRENT_DIRECTORY})
		
		# Skip interface targets because FOLDER property don't work on them
		get_target_property(_target_type ${_target} TYPE )
		if (NOT ${_target_type} MATCHES "INTERFACE_LIBRARY")		
			set_target_properties(${_target} PROPERTIES FOLDER ${CONTAINER_FOLDER}/${_relative_path})
		endif()
		
	endforeach()
endfunction()

add_subdirectory(third_party)
add_subdirectory(src)

target_include_directories(TestFlightSimulator PUBLIC ${PROJECT_ROOT}/third_party/VulkanMemoryAllocator/src)
