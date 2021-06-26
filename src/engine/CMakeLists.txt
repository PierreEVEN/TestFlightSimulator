
file(GLOB_RECURSE SOURCES *.cpp *.h)
add_library(GameEngine STATIC ${SOURCES})
configure_project(GameEngine "${SOURCES}")

# VMA
target_include_directories(GameEngine PUBLIC ${VulkanMemoryAllocator_SOURCE_DIR}/include)

# VULKAN
set(VULKAN_PATH $ENV{VULKAN_SDK})
target_include_directories(GameEngine PUBLIC ${VULKAN_PATH}/Include)
IF (WIN32)
	target_link_libraries(GameEngine ${VULKAN_PATH}/Lib/vulkan-1.lib)
ELSE()
	target_link_libraries(GameEngine -lvulkan)
ENDIF()

# Dependencies
target_link_libraries(GameEngine Utils)
target_link_libraries(GameEngine JobSystem)
target_link_libraries(GameEngine assimp)
target_link_libraries(GameEngine glfw)
target_link_libraries(GameEngine cpputils)
target_link_libraries(GameEngine spirv-cross-core)
target_link_libraries(GameEngine spirv-cross-glsl)
target_link_libraries(GameEngine glslang OSDependent OGLCompiler SPIRV)
target_link_libraries(GameEngine glm)
target_link_libraries(GameEngine imgui_interface)
target_include_directories(GameEngine PUBLIC ${stb_SOURCE_DIR})

target_include_directories(GameEngine PUBLIC public)