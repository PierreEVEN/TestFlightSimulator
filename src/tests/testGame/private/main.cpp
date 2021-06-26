#include "cpputils/logger.hpp"
#include "gameEngine.h"
#include "testGameInterface.h"

int main(int argc, char* argv[])
{
    Logger::get().enable_logs(Logger::LOG_LEVEL_TRACE | Logger::LOG_LEVEL_DEBUG | Logger::LOG_LEVEL_INFO);
    Logger::get().set_thread_identifier([]() -> uint8_t {
        return job_system::;
    });

    GameEngine::init();
    IEngineInterface::run<TestGameInterface>();
    GameEngine::cleanup();
}
