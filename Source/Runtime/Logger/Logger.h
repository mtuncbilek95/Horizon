#pragma once

#include <format>
#include <string_view>

namespace Horizon
{
    enum class LogType
    {
        Info,
        Debug,
        Success,
        Warning,
        Error,
        Fatal
    };

	struct Log
	{
	public:
		template<typename... Args>
		static void Terminal(LogType type, std::string_view category, std::format_string<Args...> fmt, Args&&... args)
		{
			Print(type, category, std::format(fmt, std::forward<Args>(args)...));
		}

		static void Terminal(LogType type, std::string_view category, std::string_view message)
		{
			Print(type, category, message);
		}

	private:
		static void Print(LogType type, std::string_view category, std::string_view message);
	};
}
