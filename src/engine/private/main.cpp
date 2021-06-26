
#include "assets/Scene.h"
#include "assets/shader.h"
#include "assets/texture2d.h"
#include "config.h"
#include "ios/scene_importer.h"
#include "jobSystem/job_system.h"
#include "misc/capabilities.h"
#include "rendering/window.h"
#include "statsRecorder.h"
#include "ui/window/windows/contentBrowser.h"
#include "ui/window/windows/profiler.h"

void window_test(bool imgui_context)
{
}

void execute()
{
    // Create workers
    job_system::Worker::create_workers();

    // Initialize rendering window
    LOG_INFO("initialize rendering");
    glfwInit();
    capabilities::check_all();
    vulkan_common::vulkan_init();

    // Create two test windows
    auto main_task = job_system::new_job([] { window_test(true); });

    // Wait remaining job completion
    LOG_INFO("waiting remaining jobs...");
    main_task->wait();

    // Destroy rendering window
    vulkan_common::vulkan_shutdown();
    glfwTerminate();
    job_system::Worker::destroy_workers();
    LOG_VALIDATE("process complete !");
}

void execute_job_test()
{
    job_system::Worker::create_workers();

    LOG_VALIDATE("run test");

    job_system::new_job([] {
        LOG_INFO("add child task");
        job_system::new_job([] {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            LOG_INFO("finnished child execution");
        })->wait();

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        job_system::new_job([] {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            LOG_INFO("finnished child execution");
        })->wait();

        LOG_INFO("wait child task");
        LOG_INFO("complete child task");
    });

    job_system::Worker::wait_job_completion();

    LOG_VALIDATE("job complete - destroying jobs");

    job_system::Worker::destroy_workers();
}

int main(int argc, char* argv[])
{
    Logger::get().set_log_file("./saved/log/Log - %s.log");
    Logger::get().enable_logs(Logger::LOG_LEVEL_TRACE | Logger::LOG_LEVEL_DEBUG | Logger::LOG_LEVEL_INFO);

    Profiler::get().begin_record();

    ShaderModule::initialize_glslang();

    try
    {
        BEGIN_RECORD();
        execute();
        // execute_job_test();
    }
    catch (std::exception& e)
    {
        LOG_FATAL("application crashed : %s", e.what());
    }

    Profiler::get().end_record();

    ShaderModule::shutdown_glslang();
}