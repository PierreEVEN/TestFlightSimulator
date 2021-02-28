
#include "config.h"
#include "assets/shader.h"
#include "engine/jobSystem/job_system.h"
#include "ios/logger.h"
#include "misc/capabilities.h"
#include "rendering/window.h"

void load_assets(Window& context)
{
	new Shader(&context, "shader_Test", "data/test.vs.glsl", "data/test.fs.glsl", "data/test.gs.glsl");
}


void window_test(bool imgui_context)
{	
	Window game_window(800, 600, config::application_name, false, imgui_context);

	job_system::new_job([&] {load_assets(game_window); });
	
	while (game_window.begin_frame()) {
		game_window.end_frame();
	}
}



int main(int argc, char* argv[])
{
	// Create workers
	job_system::Worker::create_workers();

	// Initialize rendering window
	logger_log("initialize rendering");
	glfwInit();
	capabilities::check_all();
	vulkan_common::vulkan_init();
		
	// Create two test windows
	job_system::new_job([] {window_test(true); });

	// Wait remaining job completion
	logger_log("waiting remaining jobs...");
	job_system::Worker::wait_job_completion();
	
	// Destroy rendering window
	vulkan_common::vulkan_shutdown();
	glfwTerminate();
	
	job_system::Worker::destroy_workers();
	logger_validate("process complete !");
}