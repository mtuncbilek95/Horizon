#pragma once

#include <string>
#include <string_view>
#include <format>
#include <filesystem>

namespace Horizon
{
	class Terminal
	{
		enum class Level
		{
			Log,
			Info,
			Debug,
			Warning,
			Error,
			Fatal
		};

	public:
		template<typename... Args>
		static void Log(std::string_view titleName, std::format_string<Args...> formatString, Args&&... args)
		{
			std::string message = std::format(formatString, std::forward<Args>(args)...);
			Print(Level::Log, titleName, message);
		}

		template<typename... Args>
		static void Info(std::string_view titleName, std::format_string<Args...> formatString, Args&&... args)
		{
			std::string message = std::format(formatString, std::forward<Args>(args)...);
			Print(Level::Info, titleName, message);
		}

		template<typename... Args>
		static void Debug(std::string_view titleName, std::format_string<Args...> formatString, Args&&... args)
		{
			std::string message = std::format(formatString, std::forward<Args>(args)...);
			Print(Level::Debug, titleName, message);
		}

		template<typename... Args>
		static void Warn(std::string_view titleName, std::format_string<Args...> formatString, Args&&... args)
		{
			std::string message = std::format(formatString, std::forward<Args>(args)...);
			Print(Level::Warning, titleName, message);
		}

		template<typename... Args>
		static void Error(std::string_view titleName, std::format_string<Args...> formatString, Args&&... args)
		{
			std::string message = std::format(formatString, std::forward<Args>(args)...);
			Print(Level::Error, titleName, message);
		}

		template<typename... Args>
		static void Fatal(std::string_view titleName, std::format_string<Args...> formatString, Args&&... args)
		{
			std::string message = std::format(formatString, std::forward<Args>(args)...);
			Print(Level::Fatal, titleName, message);
		}

		template<typename... Args>
		static void Assert(b8 result, std::string_view titleName, std::format_string<Args...> formatString, Args&&... args)
		{
			if (result)
				return;

			std::string message = std::format(formatString, std::forward<Args>(args)...);
			IntendedAssert(result, titleName, message);
		}

	private:
		static void Print(Level logLevel, std::string_view titleName, std::string_view message);
		static void IntendedAssert(b8 result, std::string_view titleName, std::string_view message);
	};
}

template <>
struct std::formatter<std::filesystem::path> : std::formatter<std::string> {
	auto format(const std::filesystem::path& p, std::format_context& ctx) const {
		return std::formatter<std::string>::format(p.string(), ctx);
	}
};