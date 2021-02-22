


#include <thread>

#include "config.h"
#include "engine/jobSystem/job_system.h"
#include "ios/logger.h"
#include "misc/capabilities.h"
#include "rendering/Window.h"

void window_test()
{

	Window game_window(800, 600, config::application_name);

	while (game_window.begin_frame()) {



		game_window.end_frame();
	}
}

int main(int argc, char* argv[])
{
	job_system::Worker::create_workers(-1);

	


	job_system::new_job([]()
		{
			glfwInit();
			logger::log("initialized glfw");
			capabilities::check_all();
			vulkan_common::vulkan_init();


			std::thread th(window_test);
			std::thread th2(window_test);

			th.join();
			th2.join();

			glfwTerminate();
		});
	
}
