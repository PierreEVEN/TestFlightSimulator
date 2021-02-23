#include "ios/logger.h"

#include <iostream>
#include <mutex>

#if _WIN32
#include <windows.h>
HANDLE h_console_out = GetStdHandle(STD_OUTPUT_HANDLE);
#endif

namespace logger
{
	std::mutex logger_lock;
	
	void log_print(int color, const std::string& message)
	{
		
		std::lock_guard<std::mutex> lock(logger_lock);
#if _WIN32
		SetConsoleTextAttribute(h_console_out, color);
#endif

		std::cout << "LOG : " << message << std::endl;
#if _WIN32
		SetConsoleTextAttribute(h_console_out, CONSOLE_DEFAULT);
#endif
	}
}
