#pragma once

#include <print>
#include <format>
#include <string_view>
#include <chrono>

namespace Horizon
{
	struct MainLog
	{
		template<typename... Args>
		static void Terminal(std::format_string<Args...> formatString, Args&&... args)
		{
			Print("", formatString, std::forward<Args>(args)...);
		}

		template<typename... Args>
		static void Info(std::format_string<Args...> formatString, Args&&... args)
		{
			Print("INFO", formatString, std::forward<Args>(args)...);
		}

		template<typename... Args>
		static void Debug(std::format_string<Args...> formatString, Args&&... args)
		{
			Print("DEBUG", formatString, std::forward<Args>(args)...);
		}

		template<typename... Args>
		static void Warn(std::format_string<Args...> formatString, Args&&... args)
		{
			Print("WARN", formatString, std::forward<Args>(args)...);
		}

		template<typename... Args>
		static void Error(std::format_string<Args...> formatString, Args&&... args)
		{
			Print("ERROR", formatString, std::forward<Args>(args)...);
		}

		template<typename... Args>
		static void Fatal(std::format_string<Args...> formatString, Args&&... args)
		{
			Print("FATAL", formatString, std::forward<Args>(args)...);
		}

	private:
		template<typename... Args>
		static void Print(std::string_view level, std::format_string<Args...> formatString, Args&&... args)
		{
			auto timestamp = std::chrono::system_clock::now();
			auto msg = std::format(formatString, std::forward<Args>(args)...);
			std::println("[{}] [{}] {}", timestamp, level, msg);
		}
	};
}