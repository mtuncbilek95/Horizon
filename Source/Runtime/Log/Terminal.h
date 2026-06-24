#pragma once

#include <Runtime/Containers/StringView.h>

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
		static void Log(StringView titleName, std::format_string<Args...> formatString, Args&&... args)
		{
			String message = std::format(formatString, std::forward<Args>(args)...).c_str();
			Print(Level::Log, titleName, message);
		}

		template<typename... Args>
		static void Info(StringView titleName, std::format_string<Args...> formatString, Args&&... args)
		{
			String message = std::format(formatString, std::forward<Args>(args)...).c_str();
			Print(Level::Info, titleName, message);
		}

		template<typename... Args>
		static void Debug(StringView titleName, std::format_string<Args...> formatString, Args&&... args)
		{
			String message = std::format(formatString, std::forward<Args>(args)...).c_str();
			Print(Level::Debug, titleName, message);
		}

		template<typename... Args>
		static void Warn(StringView titleName, std::format_string<Args...> formatString, Args&&... args)
		{
			String message = std::format(formatString, std::forward<Args>(args)...).c_str();
			Print(Level::Warning, titleName, message);
		}

		template<typename... Args>
		static void Error(StringView titleName, std::format_string<Args...> formatString, Args&&... args)
		{
			String message = std::format(formatString, std::forward<Args>(args)...).c_str();
			Print(Level::Error, titleName, message);
		}

		template<typename... Args>
		static void Fatal(StringView titleName, std::format_string<Args...> formatString, Args&&... args)
		{
			String message = std::format(formatString, std::forward<Args>(args)...).c_str();
			Print(Level::Fatal, titleName, message);
		}

		template<typename... Args>
		static void Assert(b8 result, StringView titleName, std::format_string<Args...> formatString, Args&&... args)
		{
			if (result)
				return;

			String message = std::format(formatString, std::forward<Args>(args)...).c_str();
			IntendedAssert(result, titleName, message);
		}

	private:
		static void Print(Level logLevel, StringView titleName, StringView message);
		static void IntendedAssert(b8 result, StringView titleName, StringView message);
	};
}
