if (NOT PkgConfig_FOUND) 
 message("-- PkgConfig is not installed - using compiled libraries")
endif()

# Vulkan
set(VULKAN_PATH $ENV{VULKAN_SDK} CACHE INTERNAL "vulkan_sdk_path")
add_library(vulkan INTERFACE)
target_include_directories(vulkan INTERFACE ${VULKAN_PATH}/Include)
IF (WIN32)
	target_link_libraries(vulkan INTERFACE ${VULKAN_PATH}/Lib/vulkan-1.lib)
ELSE()
	target_link_libraries(vulkan INTERFACE -lvulkan)
ENDIF()

message("-- vulkan sdk : " ${VULKAN_PATH})


# Vma
set(VMA_DYNAMIC_VULKAN_FUNCTIONS OFF)

# Glfw
set(BUILD_SHARED_LIBS ON)
set(GLFW_BUILD_EXAMPLES OFF)
set(GLFW_BUILD_TESTS OFF)
set(GLFW_BUILD_DOCS OFF)
set(GLFW_INSTALL OFF)

# spirv cross
set(SPIRV_CROSS_SHARED OFF)
set(SPIRV_CROSS_STATIC ON)
set(SPIRV_CROSS_CLI OFF)
set(SPIRV_CROSS_ENABLE_TESTS OFF)
set(SPIRV_CROSS_ENABLE_HLSL OFF)
set(SPIRV_CROSS_ENABLE_MSL OFF)
set(SPIRV_CROSS_ENABLE_CPP OFF)
set(SPIRV_CROSS_ENABLE_C_API OFF)
set(SPIRV_CROSS_ENABLE_UTIL OFF)
set(SPIRV_CROSS_ENABLE_REFLECT OFF)
set(SPIRV_CROSS_SKIP_INSTALL ON)
set(SPIRV_CROSS_SKIP_INSTALL ON)
set(SPIRV_CROSS_SKIP_INSTALL ON)

# spirv reflect
set(SPIRV_REFLECT_BUILD_TESTS OFF)

# Assimp
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF)
set(ASSIMP_BUILD_TESTS OFF)
set(ASSIMP_NO_EXPORT ON)
set(ASSIMP_INSTALL OFF)
set(ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT OFF)
set(ASSIMP_BUILD_GLTF_IMPORTER ON)

# Glslang
set(BUILD_SHARED_LIBS OFF)
set(ENABLE_HLSL OFF)
set(SKIP_GLSLANG_INSTALL ON)
set(ENABLE_GLSLANG_BINARIES OFF)

fetch_target(glfw https://github.com/glfw/glfw master)
fetch_target(assimp https://github.com/assimp/assimp master)
fetch_target(SPIRV-Cross https://github.com/KhronosGroup/SPIRV-Cross master)
fetch_target(glslang https://github.com/KhronosGroup/glslang master)
fetch_target(stb https://github.com/nothings/stb master)
fetch_target(glm https://github.com/g-truc/glm master)
fetch_target(VulkanMemoryAllocator https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator  master)
fetch_target(cpputils https://github.com/PierreEVEN/CppUtils main)
fetch_target(imgui https://github.com/ocornut/imgui docking)

# Create an interface for IMGUI (should alway be linked as static library)
file(GLOB IMGUI_SOURCES 
	${imgui_SOURCE_DIR}/*.cpp
	${imgui_SOURCE_DIR}/*.h
	${imgui_SOURCE_DIR}/backends/imgui_impl_glfw*)
add_library(imgui_interface STATIC ${IMGUI_SOURCES})
target_include_directories(imgui_interface PUBLIC ${imgui_SOURCE_DIR})
target_link_libraries(imgui_interface vulkan)
target_link_libraries(imgui_interface glfw)

