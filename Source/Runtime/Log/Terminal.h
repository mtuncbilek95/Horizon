#pragma once

#include <string_view>
#include <format>

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
			Print(Level::Log, titleName, std::format(formatString, std::forward<Args>(args)...));
		}

		template<typename... Args>
		static void Info(std::string_view titleName, std::format_string<Args...> formatString, Args&&... args)
		{
			Print(Level::Info, titleName, std::format(formatString, std::forward<Args>(args)...));
		}

		template<typename... Args>
		static void Debug(std::string_view titleName, std::format_string<Args...> formatString, Args&&... args)
		{
			Print(Level::Debug, titleName, std::format(formatString, std::forward<Args>(args)...));
		}

		template<typename... Args>
		static void Warn(std::string_view titleName, std::format_string<Args...> formatString, Args&&... args)
		{
			Print(Level::Warning, titleName, std::format(formatString, std::forward<Args>(args)...));
		}

		template<typename... Args>
		static void Error(std::string_view titleName, std::format_string<Args...> formatString, Args&&... args)
		{
			Print(Level::Error, titleName, std::format(formatString, std::forward<Args>(args)...));
		}

		template<typename... Args>
		static void Fatal(std::string_view titleName, std::format_string<Args...> formatString, Args&&... args)
		{
			Print(Level::Fatal, titleName, std::format(formatString, std::forward<Args>(args)...));
		}

		template<typename... Args>
		static void Assert(b8 result, std::string_view titleName, std::format_string<Args...> formatString, Args&&... args)
		{
			if (result)
				return;

			IntendedAssert(result, titleName, std::format(formatString, std::forward<Args>(args)...));
		}

	private:
		static void Print(Level logLevel, std::string_view titleName, std::string_view message);
		static void IntendedAssert(b8 result, std::string_view titleName, std::string_view message);
	};
}