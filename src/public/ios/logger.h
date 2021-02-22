#pragma once
#include <memory>
#include <string>

namespace logger
{
	enum ConsoleColor {
		CONSOLE_FG_COLOR_BLACK = 0,
		CONSOLE_FG_COLOR_BLUE = 1,
		CONSOLE_FG_COLOR_GREEN = 2,
		CONSOLE_FG_COLOR_RED = 4,
		CONSOLE_FG_COLOR_LIGHT = 8,
		CONSOLE_FG_COLOR_LIGHT_RED = CONSOLE_FG_COLOR_RED | CONSOLE_FG_COLOR_LIGHT,
		CONSOLE_FG_COLOR_WHITE = CONSOLE_FG_COLOR_RED | CONSOLE_FG_COLOR_GREEN | CONSOLE_FG_COLOR_BLUE,
		CONSOLE_FG_COLOR_ORANGE = CONSOLE_FG_COLOR_RED | CONSOLE_FG_COLOR_GREEN,
		CONSOLE_FG_COLOR_VIOLET = CONSOLE_FG_COLOR_RED | CONSOLE_FG_COLOR_BLUE,
		CONSOLE_FG_COLOR_LIGHT_BLUE = CONSOLE_FG_COLOR_GREEN | CONSOLE_FG_COLOR_BLUE,
		CONSOLE_FG_COLOR_CYAN = CONSOLE_FG_COLOR_GREEN | CONSOLE_FG_COLOR_BLUE | CONSOLE_FG_COLOR_LIGHT,
		CONSOLE_BG_COLOR_BLUE = 16,
		CONSOLE_BG_COLOR_GREEN = 32,
		CONSOLE_BG_COLOR_RED = 64,
		CONSOLE_BG_COLOR_LIGHT = 128,
		CONSOLE_DEFAULT = CONSOLE_FG_COLOR_WHITE,
		CONSOLE_VALIDATE = CONSOLE_FG_COLOR_GREEN,
		CONSOLE_DISPLAY = CONSOLE_FG_COLOR_LIGHT_BLUE,
		CONSOLE_WARNING = CONSOLE_FG_COLOR_ORANGE,
		CONSOLE_FAIL = CONSOLE_FG_COLOR_LIGHT_RED,
		CONSOLE_ASSERT = CONSOLE_FG_COLOR_VIOLET | CONSOLE_BG_COLOR_RED | CONSOLE_BG_COLOR_GREEN | CONSOLE_BG_COLOR_LIGHT
	};
	
	template<typename... Params>
	std::string format(const char* format, Params... args) {
		const int size = snprintf(nullptr, 0, format, args...) + 1;
		if (size <= 0) return format;
		const std::unique_ptr<char[]> buffer(new char[size]);
		snprintf(buffer.get(), size, format, args ...);
		return std::string(buffer.get());

	}
	
	void log_print(int color, const std::string& message);

	template<typename... Params>
	void log(const char* format, Params... args)	{
		log_print(CONSOLE_DISPLAY, logger::format(format, std::forward<Params>(args)...));
	}
	
	template<typename... Params>
	void validate(const char* format, Params... args) {
		log_print(CONSOLE_VALIDATE, logger::format(format, std::forward<Params>(args)...));
	}
	
	template<typename... Params>
	void warning(const char* format, Params... args) {
		log_print(CONSOLE_WARNING, logger::format(format, std::forward<Params>(args)...));
	}
		
	template<typename... Params>
	void error(const char* format, Params... args) {
		log_print(CONSOLE_FAIL, logger::format(format, std::forward<Params>(args)...));
	}

	template<typename... Params>
	void fail(const char* format, Params... args) {
		log_print(CONSOLE_ASSERT, logger::format(format, std::forward<Params>(args)...));
		exit(EXIT_FAILURE);
	}
}
