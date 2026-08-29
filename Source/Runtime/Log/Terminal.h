#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>

#include <string>
#include <string_view>
#include <format>

namespace Horizon
{
	enum class LogLevel : u8
	{
		Debug,
		Log,
		Info,
		Warning,
		Error,
		Fatal,
		Off
	};

#if defined(HORIZON_SHIPPING)
	inline constexpr LogLevel CompiledMinLevel = LogLevel::Warning;
#else
	inline constexpr LogLevel CompiledMinLevel = LogLevel::Debug;
#endif

	class H_EXPORT ILogSink
	{
	public:
		virtual ~ILogSink() = default;
		virtual void OnMessage(LogLevel level, std::string_view titleName, std::string_view message) = 0;
	};

	class H_EXPORT Terminal
	{
	public:
		static void SetMinLevel(LogLevel level);
		static LogLevel GetMinLevel();

		static void AddSink(ILogSink* pSink);
		static void RemoveSink(ILogSink* pSink);

		static b8 IsEnabled(LogLevel level)
		{
			if constexpr (CompiledMinLevel == LogLevel::Off)
				return false;

			return level >= CompiledMinLevel && level >= GetMinLevel();
		}

		template<typename... Args>
		static void Debug(std::string_view titleName, std::format_string<Args...> formatString, Args&&... args)
		{
			Emit(LogLevel::Debug, titleName, formatString, std::forward<Args>(args)...);
		}

		template<typename... Args>
		static void Log(std::string_view titleName, std::format_string<Args...> formatString, Args&&... args)
		{
			Emit(LogLevel::Log, titleName, formatString, std::forward<Args>(args)...);
		}

		template<typename... Args>
		static void Info(std::string_view titleName, std::format_string<Args...> formatString, Args&&... args)
		{
			Emit(LogLevel::Info, titleName, formatString, std::forward<Args>(args)...);
		}

		template<typename... Args>
		static void Warn(std::string_view titleName, std::format_string<Args...> formatString, Args&&... args)
		{
			Emit(LogLevel::Warning, titleName, formatString, std::forward<Args>(args)...);
		}

		template<typename... Args>
		static void Error(std::string_view titleName, std::format_string<Args...> formatString, Args&&... args)
		{
			Emit(LogLevel::Error, titleName, formatString, std::forward<Args>(args)...);
		}

		template<typename... Args>
		static void Fatal(std::string_view titleName, std::format_string<Args...> formatString, Args&&... args)
		{
			std::string message = std::format(formatString, std::forward<Args>(args)...);
			Print(LogLevel::Fatal, titleName, message);
			Abort(titleName, message);
		}

		template<typename... Args>
		static void Assert(b8 result, std::string_view titleName, std::format_string<Args...> formatString, Args&&... args)
		{
			if (result)
				return;

			std::string message = std::format(formatString, std::forward<Args>(args)...);
			Print(LogLevel::Fatal, titleName, message);
			Abort(titleName, message);
		}

	private:
		template<typename... Args>
		static void Emit(LogLevel level, std::string_view titleName, std::format_string<Args...> formatString, Args&&... args)
		{
			if (!IsEnabled(level))
				return;

			std::string message = std::format(formatString, std::forward<Args>(args)...);
			Print(level, titleName, message);
		}

		static void Print(LogLevel level, std::string_view titleName, std::string_view message);
		[[noreturn]] static void Abort(std::string_view titleName, std::string_view message);
	};
}