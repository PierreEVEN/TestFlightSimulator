
file(GLOB_RECURSE SOURCES *.cpp *.h)
add_executable(TestFlightSimulator ${SOURCES})
configure_project(TestFlightSimulator "${SOURCES}")


# VMA
target_include_directories(TestFlightSimulator PUBLIC ${VulkanMemoryAllocator_SOURCE_DIR}/include)

# VULKAN
set(VULKAN_PATH $ENV{VULKAN_SDK})
target_include_directories(TestFlightSimulator PUBLIC ${VULKAN_PATH}/Include)
IF (WIN32)
	target_link_libraries(TestFlightSimulator ${VULKAN_PATH}/Lib/vulkan-1.lib)
ELSE()
	target_link_libraries(TestFlightSimulator -lvulkan)
ENDIF()

# Dependencies
target_link_libraries(TestFlightSimulator Utils)
target_link_libraries(TestFlightSimulator JobSystem)
target_link_libraries(TestFlightSimulator assimp)
target_link_libraries(TestFlightSimulator glfw)
target_link_libraries(TestFlightSimulator cpputils)
target_link_libraries(TestFlightSimulator spirv-cross-core)
target_link_libraries(TestFlightSimulator spirv-cross-glsl)
target_link_libraries(TestFlightSimulator glslang OSDependent OGLCompiler SPIRV)


target_include_directories(TestFlightSimulator PUBLIC public)

# GLM
target_link_libraries(TestFlightSimulator glm)

# STB
target_include_directories(TestFlightSimulator PUBLIC ${stb_SOURCE_DIR})

