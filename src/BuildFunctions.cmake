include(FetchContent)

function(configure_project PROJECT_NAME)
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

function(configure_target TARGET_NAME OUTPUT_FOLDER)
	if (TARGET ${TARGET_NAME})
		# Skip interface targets because FOLDER property don't work on them
		get_target_property(_target_type ${TARGET_NAME} TYPE )
		if (NOT ${_target_type} MATCHES "INTERFACE_LIBRARY")
			set_target_properties(${TARGET_NAME} PROPERTIES 
				ARCHIVE_OUTPUT_DIRECTORY "${BINARIES_DIR}/Lib"
				RUNTIME_OUTPUT_DIRECTORY "${BINARIES_DIR}/Bin"
				LIBRARY_OUTPUT_DIRECTORY "${BINARIES_DIR}/Bin"
				VS_DEBUGGER_WORKING_DIRECTORY "${PROJECT_ROOT}")	
			set_target_properties(${TARGET_NAME} PROPERTIES FOLDER ${OUTPUT_FOLDER})
		endif()
	endif()
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
		configure_target(${_target} ${CONTAINER_FOLDER}/${_relative_path})		
	endforeach()
endfunction()



function(fetch_target TARGET_NAME URL BRANCH)
	message("--- [fetching ${TARGET_NAME}]")
	FetchContent_Declare(
	  ${TARGET_NAME}
	  GIT_REPOSITORY  ${URL}
	  GIT_TAG ${BRANCH}
	)
	FetchContent_MakeAvailable(${TARGET_NAME})	
	configure_target(${TARGET_NAME} third_party)		
	
	
	get_property(target_src_dir VARIABLE PROPERTY ${TARGET_NAME}_SOURCE_DIR)	
	set(${TARGET_NAME}_SOURCE_DIR ${target_src_dir} CACHE INTERNAL "source_list")

endfunction()
