
#include "config.h"
#include "assets/Scene.h"
#include "assets/shader.h"
#include "statsRecorder.h"
#include "assets/texture2d.h"
#include "jobSystem/job_system.h"
#include "ios/logger.h"
#include "ios/scene_importer.h"
#include "misc/capabilities.h"
#include "rendering/window.h"
#include "ui/window/windows/contentBrowser.h"
#include "ui/window/windows/profiler.h"

void window_test(bool imgui_context)
{	
	Window game_window(800, 600, config::application_name, false, imgui_context);
	

	game_window.get_asset_manager()->create<Scene>("F-16", "data/F-16_b.glb");
	
	auto default_texture = game_window.get_asset_manager()->create<Texture2d>("default-texture", "data/DefaultTexture.png");

	auto shader = game_window.get_asset_manager()->create<Shader>(
	"shader_Test",
		"data/test.vs.glsl",
		"data/test.fs.glsl"
		);

	new ProfilerWindow(&game_window, "profiler");
	new ContentBrowser(&game_window, "content browser");
		
	while (game_window.begin_frame()) {
		game_window.end_frame();
	}
}

void execute()
{
	// Create workers
	job_system::Worker::create_workers();

	// Initialize rendering window
	logger_log("initialize rendering");
	glfwInit();
	capabilities::check_all();
	vulkan_common::vulkan_init();

	// Create two test windows
	auto main_task = job_system::new_job([] {window_test(true); });

	// Wait remaining job completion
	logger_log("waiting remaining jobs...");
	main_task->wait();
	
	// Destroy rendering window
	vulkan_common::vulkan_shutdown();
	glfwTerminate();
	job_system::Worker::destroy_workers();
	logger_validate("process complete !");
}


void execute_job_test()
{
	job_system::Worker::create_workers();

	logger_validate("run test");

	job_system::new_job([]
		{
			logger_log("add child task");
			job_system::new_job([]
				{

					std::this_thread::sleep_for(std::chrono::milliseconds(500));
					logger_log("finnished child execution");
				
				})->wait();
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			job_system::new_job([]
				{

					std::this_thread::sleep_for(std::chrono::milliseconds(500));
					logger_log("finnished child execution");

				})->wait();
			logger_log("wait child task");
			logger_log("complete child task");
		});

	job_system::Worker::wait_job_completion();

	logger_validate("job complete - destroying jobs");
	
	job_system::Worker::destroy_workers();
}


int main(int argc, char* argv[])
{
	Profiler::get().begin_record();

	try
	{
		BEGIN_RECORD();
		execute();
		//execute_job_test();
	}
	catch (std::exception& e)
	{
		logger_fail("application crashed : %s", e.what());
	}
	
	Profiler::get().end_record();
}