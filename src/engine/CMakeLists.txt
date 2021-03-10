
file(GLOB_RECURSE SOURCES *.cpp *.h)
add_executable(TestFlightSimulator ${SOURCES})
construct_target(TestFlightSimulator . ${SOURCES})

# Dependencies
target_link_libraries(TestFlightSimulator Utils)
target_link_libraries(TestFlightSimulator JobSystem)

# ASSIMP
target_include_directories(TestFlightSimulator PUBLIC ${PROJECT_ROOT}/third_party/assimp/include)
target_include_directories(TestFlightSimulator PUBLIC ${PROJECT_ROOT}/third_party/assimp/Build/include)
IF (WIN32)
	target_link_libraries(TestFlightSimulator $<$<CONFIG:Release>:${PROJECT_ROOT}/temp/third_party/release/assimp-vc142-mt.lib> $<$<CONFIG:Debug>:${PROJECT_ROOT}/temp/third_party/debug/assimp-vc142-mtd.lib>)
	target_link_libraries(TestFlightSimulator $<$<CONFIG:Release>:${PROJECT_ROOT}/temp/third_party/release/zlibstatic.lib> $<$<CONFIG:Debug>:${PROJECT_ROOT}/temp/third_party/debug/zlibstaticd.lib>)
ELSE()
ENDIF()

# SPIRV-CROSS
target_include_directories(TestFlightSimulator PUBLIC ${PROJECT_ROOT}/third_party/SPIRV-cross)
IF (WIN32)
	target_link_libraries(TestFlightSimulator $<$<CONFIG:Release>:${PROJECT_ROOT}/temp/third_party/release/spirv-cross-core.lib> $<$<CONFIG:Debug>:${PROJECT_ROOT}/temp/third_party/debug/spirv-cross-cored.lib>)
	target_link_libraries(TestFlightSimulator $<$<CONFIG:Release>:${PROJECT_ROOT}/temp/third_party/release/spirv-cross-glsl.lib> $<$<CONFIG:Debug>:${PROJECT_ROOT}/temp/third_party/debug/spirv-cross-glsld.lib>)
ELSE()
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

# GLM
target_include_directories(TestFlightSimulator PUBLIC ${PROJECT_ROOT}/third_party/glm)

# STB
target_include_directories(TestFlightSimulator PUBLIC ${PROJECT_ROOT}/third_party/stb)

# GLFW
target_include_directories(TestFlightSimulator PUBLIC ${PROJECT_ROOT}/third_party/glfw/include)
IF (WIN32)
	target_link_libraries(TestFlightSimulator $<$<CONFIG:Release>:${PROJECT_ROOT}/temp/third_party/release/glfw3.lib> $<$<CONFIG:Debug>:${PROJECT_ROOT}/temp/third_party/debug/glfw3.lib>)
ELSE()
ENDIF()

# SHADERC
target_include_directories(TestFlightSimulator PUBLIC ${PROJECT_ROOT}/third_party/shaderc/libshaderc/Include)
IF (WIN32)
	target_link_libraries(TestFlightSimulator $<$<CONFIG:Release>:${PROJECT_ROOT}/temp/third_party/release/shaderc.lib> $<$<CONFIG:Debug>:${PROJECT_ROOT}/temp/third_party/debug/shaderc.lib>)
	target_link_libraries(TestFlightSimulator $<$<CONFIG:Release>:${PROJECT_ROOT}/temp/third_party/release/shaderc_util.lib> $<$<CONFIG:Debug>:${PROJECT_ROOT}/temp/third_party/debug/shaderc_util.lib>)
	target_link_libraries(TestFlightSimulator $<$<CONFIG:Release>:${PROJECT_ROOT}/temp/third_party/release/GenericCodeGen.lib> $<$<CONFIG:Debug>:${PROJECT_ROOT}/temp/third_party/debug/GenericCodeGend.lib>)
	target_link_libraries(TestFlightSimulator $<$<CONFIG:Release>:${PROJECT_ROOT}/temp/third_party/release/MachineIndependent.lib> $<$<CONFIG:Debug>:${PROJECT_ROOT}/temp/third_party/debug/MachineIndependentd.lib>)
	target_link_libraries(TestFlightSimulator $<$<CONFIG:Release>:${PROJECT_ROOT}/temp/third_party/release/OGLCompiler.lib> $<$<CONFIG:Debug>:${PROJECT_ROOT}/temp/third_party/debug/OGLCompilerd.lib>)
	target_link_libraries(TestFlightSimulator $<$<CONFIG:Release>:${PROJECT_ROOT}/temp/third_party/release/OSDependent.lib> $<$<CONFIG:Debug>:${PROJECT_ROOT}/temp/third_party/debug/OSDependentd.lib>)
	target_link_libraries(TestFlightSimulator $<$<CONFIG:Release>:${PROJECT_ROOT}/temp/third_party/release/glslang.lib> $<$<CONFIG:Debug>:${PROJECT_ROOT}/temp/third_party/debug/glslangd.lib>)
	target_link_libraries(TestFlightSimulator $<$<CONFIG:Release>:${PROJECT_ROOT}/temp/third_party/release/SPIRV.lib> $<$<CONFIG:Debug>:${PROJECT_ROOT}/temp/third_party/debug/SPIRVd.lib>)
	target_link_libraries(TestFlightSimulator $<$<CONFIG:Release>:${PROJECT_ROOT}/temp/third_party/release/SPIRV-Tools.lib> $<$<CONFIG:Debug>:${PROJECT_ROOT}/temp/third_party/debug/SPIRV-Tools.lib>)
	target_link_libraries(TestFlightSimulator $<$<CONFIG:Release>:${PROJECT_ROOT}/temp/third_party/release/SPIRV-Tools-opt.lib> $<$<CONFIG:Debug>:${PROJECT_ROOT}/temp/third_party/debug/SPIRV-Tools-opt.lib>)
	
ELSE()
ENDIF()
