# Glfw
add_subfodler_target(glfw ThirdParty)

set(BUILD_SHARED_LIBS ON)
set(ASSIMP_NO_EXPORT ON)
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF)
set(ASSIMP_BUILD_TESTS OFF)
set(ASSIMP_INSTALL OFF)
set(ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT OFF)
add_subfodler_target(assimp ThirdParty)
add_subfodler_target(SPIRV-Cross ThirdParty)
add_subfodler_target(CppUtils ThirdParty)
add_subfodler_target(glm ThirdParty)

# ShaderC
message("-- updating shaderc dependencies ... ")
execute_process(COMMAND python ${CMAKE_CURRENT_SOURCE_DIR}/shaderc/utils/git-sync-deps)
set(SHADERC_ENABLE_SHARED_CRT true)
add_subfodler_target(shaderc ThirdParty)


# Vulkan
set(VULKAN_PATH $ENV{VULKAN_SDK})
message("-- vulkan sdk : " ${VULKAN_PATH})
add_library(vulkan INTERFACE)
target_include_directories(vulkan INTERFACE ${VULKAN_PATH}/Include)
IF (WIN32)
	target_link_libraries(vulkan INTERFACE ${VULKAN_PATH}/Lib/vulkan-1.lib)
ELSE()
	target_link_libraries(vulkan INTERFACE -lvulkan)
ENDIF()