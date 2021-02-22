#include "ios/logger.h"

#include <iostream>

#if _WIN32
#include <windows.h>
HANDLE h_console_out = GetStdHandle(STD_OUTPUT_HANDLE);
#endif

namespace logger
{
	void log_print(int color, const std::string& message)
	{
#if _WIN32
		SetConsoleTextAttribute(h_console_out, color);
#endif

		std::cout << "LOG : " << message << std::endl;
#if _WIN32
		SetConsoleTextAttribute(h_console_out, CONSOLE_DEFAULT);
#endif
	}
}
