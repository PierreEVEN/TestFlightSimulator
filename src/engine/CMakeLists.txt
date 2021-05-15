
file(GLOB_RECURSE SOURCES *.cpp *.h)
add_executable(TestFlightSimulator ${SOURCES})
construct_target(TestFlightSimulator . ${SOURCES})


# VMA
target_include_directories(TestFlightSimulator PUBLIC ${PROJECT_ROOT}/third_party/VulkanMemoryAllocator/src)

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
target_link_libraries(TestFlightSimulator shaderc)
target_link_libraries(TestFlightSimulator shaderc_util)
target_link_libraries(TestFlightSimulator GenericCodeGen)
target_link_libraries(TestFlightSimulator MachineIndependent)
target_link_libraries(TestFlightSimulator OGLCompiler)
target_link_libraries(TestFlightSimulator OSDependent)
target_link_libraries(TestFlightSimulator glslang)
target_link_libraries(TestFlightSimulator SPIRV)
target_link_libraries(TestFlightSimulator SPIRV-Tools)
target_link_libraries(TestFlightSimulator SPIRV-Tools-opt)

target_include_directories(TestFlightSimulator PUBLIC public)

# GLM
target_include_directories(TestFlightSimulator PUBLIC ${PROJECT_ROOT}/third_party/glm)

# STB
target_include_directories(TestFlightSimulator PUBLIC ${PROJECT_ROOT}/third_party/stb)

