#include "Terminal.h"

#include <print>
#include <chrono>
#include <cassert>

namespace Horizon
{
	void Terminal::Print(Level logLevel, std::string_view titleName, std::string_view message)
	{
		constexpr const char* kOrange = "\033[38;5;208m";
		constexpr const char* kReset = "\033[0m";

		const char* color = "\033[90m";
		const char* name = "Log";

		switch (logLevel)
		{
		case Level::Log:
			color = "\033[90m";
			name = "Log";
			break;
		case Level::Info:
			color = "\033[32m";
			name = "Info";
			break;
		case Level::Debug:
			color = "\033[36m";
			name = "Debug";
			break;
		case Level::Warning:
			color = "\033[33m";
			name = "Warning";
			break;
		case Level::Error:
			color = "\033[31m";
			name = "Error";
			break;
		case Level::Fatal:
			color = "\033[35m";
			name = "Fatal";
			break;
		}

		const auto now = std::chrono::current_zone()->to_local(std::chrono::system_clock::now());
		const auto time = std::chrono::floor<std::chrono::seconds>(now);

		std::println("[{:%H:%M:%S}][{}{}{}][{}{}{}]: {}", time, kOrange, titleName, kReset, 
			color, name, kReset, message);
	}

	void Terminal::IntendedAssert(b8 result, std::string_view titleName, std::string_view message)
	{
		Print(Level::Fatal, titleName, message);
		assert(result);
	}

}
