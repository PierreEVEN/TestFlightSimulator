
#include "config.h"
#include "assets/shader.h"
#include "engine/jobSystem/job_system.h"
#include "ios/logger.h"
#include "ios/scene_importer.h"
#include "misc/capabilities.h"
#include "rendering/window.h"
#include "scene/node.h"



void window_test(bool imgui_context)
{	
	Window game_window(800, 600, config::application_name, false, imgui_context);
	
	game_window.get_asset_manager()->create<Shader>("shader_Test", "data/test.vs.glsl", "data/test.fs.glsl", "data/test.gs.glsl");
	game_window.get_asset_manager()->create<Shader>("shader_Test_1", "data/test.vs.glsl", "data/test.fs.glsl", "data/test.gs.glsl");
	game_window.get_asset_manager()->create<Shader>("shader_Test_2", "data/test.vs.glsl", "data/test.fs.glsl", "data/test.gs.glsl");
	game_window.get_asset_manager()->create<Shader>("shader_Test_3", "data/test.vs.glsl", "data/test.fs.glsl", "data/test.gs.glsl");
	game_window.get_asset_manager()->create<Shader>("shader_Test_4", "data/test.vs.glsl", "data/test.fs.glsl", "data/test.gs.glsl");
	game_window.get_asset_manager()->create<Shader>("shader_Test_5", "data/test.vs.glsl", "data/test.fs.glsl", "data/test.gs.glsl");
	game_window.get_asset_manager()->create<Shader>("shader_Test_6", "data/test.vs.glsl", "data/test.fs.glsl", "data/test.gs.glsl");
	game_window.get_asset_manager()->create<Shader>("shader_Test_7", "data/test.vs.glsl", "data/test.fs.glsl", "data/test.gs.glsl");
	game_window.get_asset_manager()->create<Shader>("shader_Test_8", "data/test.vs.glsl", "data/test.fs.glsl", "data/test.gs.glsl");
	game_window.get_asset_manager()->create<Shader>("shader_Test_9", "data/test.vs.glsl", "data/test.fs.glsl", "data/test.gs.glsl");
	game_window.get_asset_manager()->create<Shader>("shader_Test_10", "data/test.vs.glsl", "data/test.fs.glsl", "data/test.gs.glsl");
	game_window.get_asset_manager()->create<Shader>("shader_Test_11", "data/test.vs.glsl", "data/test.fs.glsl", "data/test.gs.glsl");
	game_window.get_asset_manager()->create<Shader>("shader_Test_12", "data/test.vs.glsl", "data/test.fs.glsl", "data/test.gs.glsl");
	game_window.get_asset_manager()->create<Shader>("shader_Test_13", "data/test.vs.glsl", "data/test.fs.glsl", "data/test.gs.glsl");
	game_window.get_asset_manager()->create<Shader>("shader_Test_14", "data/test.vs.glsl", "data/test.fs.glsl", "data/test.gs.glsl");
	game_window.get_asset_manager()->create<Shader>("shader_Test_15", "data/test.vs.glsl", "data/test.fs.glsl", "data/test.gs.glsl");
	game_window.get_asset_manager()->create<Shader>("shader_Test_16", "data/test.vs.glsl", "data/test.fs.glsl", "data/test.gs.glsl");

	for (int i = 0; i < 100; ++i)
		new SceneImporter (&game_window, "data/F-16_b.glb", std::to_string(i));
	
	SceneImporter importer(&game_window, "data/F-16_b.glb", "A");
	SceneImporter importer2(&game_window, "data/F-16_b.glb", "B");
	SceneImporter importer3(&game_window, "data/F-16_b.glb", "C");
	SceneImporter importer4(&game_window, "data/F-16_b.glb", "D");
	SceneImporter importer5(&game_window, "data/F-16_b.glb", "E");
	SceneImporter importer6(&game_window, "data/F-16_b.glb", "F");
	SceneImporter importer7(&game_window, "data/F-16_b.glb", "G");
	SceneImporter importer8(&game_window, "data/F-16_b.glb", "H");
	SceneImporter importer9(&game_window, "data/F-16_b.glb", "I");
	SceneImporter importer10(&game_window, "data/F-16_b.glb", "J");
	SceneImporter importer11(&game_window, "data/F-16_b.glb", "i");
	SceneImporter importer12(&game_window, "data/F-16_b.glb", "j");
	SceneImporter importer13(&game_window, "data/F-16_b.glb", "k");
	SceneImporter importer14(&game_window, "data/F-16_b.glb", "l");
	SceneImporter importer15(&game_window, "data/F-16_b.glb", "m");
	SceneImporter importer16(&game_window, "data/F-16_b.glb", "n");
	SceneImporter importer17(&game_window, "data/F-16_b.glb", "o");
	SceneImporter importer18(&game_window, "data/F-16_b.glb", "p");
	SceneImporter importer19(&game_window, "data/F-16_b.glb", "q");
	SceneImporter importer20(&game_window, "data/F-16_b.glb", "r");
	
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


	while (true) {}
}