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


	void log_print(const char* type, int color, const std::string& message, const char* function, size_t line,
		const char* file)
	{
		std::lock_guard<std::mutex> lock(logger_lock);


		struct tm time_str;
		static char time_buffer[80];
		time_t now = time(0);
		localtime_s(&time_str, &now);
		//strftime(time_buffer, sizeof(time_buffer), "%d/%b/%Y %X", &time_str);
		strftime(time_buffer, sizeof(time_buffer), "%X", &time_str);

#if _WIN32
		SetConsoleTextAttribute(h_console_out, color);
#endif

		if (function) std::cout << log_format("[%s] [%s] %s::%d : %s", time_buffer, type, function, line, message.c_str());
		else std::cout << log_format("[%s] [%s] : %s", time_buffer, type, message.c_str());

		if (file) std::cout << log_format("\n\t=>%s", file);
		
		std::cout << std::endl;
#if _WIN32
		SetConsoleTextAttribute(h_console_out, CONSOLE_DEFAULT);
#endif
	}
}
