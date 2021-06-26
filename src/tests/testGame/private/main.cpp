#include "cpputils/logger.hpp"
#include "gameEngine.h"
#include "testGameInterface.h"

int main(int argc, char* argv[])
{
    Logger::get().enable_logs(Logger::LOG_LEVEL_TRACE | Logger::LOG_LEVEL_DEBUG | Logger::LOG_LEVEL_INFO);

    GameEngine::init();
    IEngineInterface::run<TestGameInterface>();
    GameEngine::cleanup();
}
