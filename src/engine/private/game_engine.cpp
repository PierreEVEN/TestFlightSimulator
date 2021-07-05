#include "game_engine.h"

#include "cpputils/logger.hpp"
#include "GLFW/glfw3.h"
#include "glslang/Include/glslang_c_interface.h"
#include "jobSystem/worker.h"
#include "misc/capabilities.h"
#include "rendering/vulkan/common.h"


namespace GameEngine
{

void init()
{
    Logger::get().set_thread_identifier([]() -> uint8_t {
        if (auto* found_worker = job_system::Worker::get())
            return found_worker->get_worker_id();
        return UINT8_MAX;
    });
    Logger::get().set_log_file("./saved/log/Log - %s.log");
    LOG_INFO("Initialize game engine");
    glslang_initialize_process();

    job_system::Worker::create_workers();

    LOG_INFO("initialize rendering");
    glfwInit();
    capabilities::check_all();
    vulkan_common::vulkan_init();
}

void cleanup()
{
    LOG_INFO("CLeanup game engine");

    // Destroy rendering window
    vulkan_common::vulkan_shutdown();
    glfwTerminate();
    job_system::Worker::destroy_workers();
    glslang_finalize_process();
}

} // namespace GameEngine