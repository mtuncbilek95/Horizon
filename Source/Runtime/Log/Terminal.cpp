#include "Terminal.h"

#include <Runtime/Containers/List.h>

#include <chrono>
#include <cstdlib>
#include <mutex>
#include <print>

#if defined(HORIZON_WINDOWS)
#include <io.h>
#include <Windows.h>
#endif

namespace Horizon
{
	namespace
	{
		struct SinkRegistry
		{
			List<ILogSink*> sinks;
			std::mutex mutex;
		};

		SinkRegistry& Registry()
		{
			static SinkRegistry instance;
			return instance;
		}

		LogLevel g_minLevel = LogLevel::Debug;

		std::chrono::seconds CachedUtcOffset()
		{
			static const std::chrono::seconds offset = []()
				{
					const auto now = std::chrono::system_clock::now();
					const auto info = std::chrono::current_zone()->get_info(now);
					return std::chrono::duration_cast<std::chrono::seconds>(info.offset);
				}();

			return offset;
		}

		b8 UseColor()
		{
			static const b8 enabled = []() -> b8
				{
#if defined(HORIZON_WINDOWS)
					if (_isatty(_fileno(stdout)) == 0)
						return false;

					HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
					DWORD mode = 0;

					if (!GetConsoleMode(handle, &mode))
						return false;

					SetConsoleMode(handle, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
					return true;
#else
					return false;
#endif
				}();

			return enabled;
		}

		const c8* ColorFor(LogLevel level)
		{
			switch (level)
			{
			case LogLevel::Debug:   return "\033[36m";
			case LogLevel::Log:     return "\033[90m";
			case LogLevel::Info:    return "\033[32m";
			case LogLevel::Warning: return "\033[33m";
			case LogLevel::Error:   return "\033[31m";
			case LogLevel::Fatal:   return "\033[35m";
			default:                return "\033[90m";
			}
		}

		const c8* NameFor(LogLevel level)
		{
			switch (level)
			{
			case LogLevel::Debug:   return "Debug";
			case LogLevel::Log:     return "Log";
			case LogLevel::Info:    return "Info";
			case LogLevel::Warning: return "Warning";
			case LogLevel::Error:   return "Error";
			case LogLevel::Fatal:   return "Fatal";
			default:                return "Log";
			}
		}
	}

	void Terminal::SetMinLevel(LogLevel level) { g_minLevel = level; }
	LogLevel Terminal::GetMinLevel() { return g_minLevel; }

	void Terminal::AddSink(ILogSink* pSink)
	{
		if (!pSink)
			return;

		SinkRegistry& registry = Registry();
		std::lock_guard lock(registry.mutex);
		registry.sinks.PushBack(pSink);
	}

	void Terminal::RemoveSink(ILogSink* pSink)
	{
		SinkRegistry& registry = Registry();
		std::lock_guard lock(registry.mutex);

		for (usize i = 0; i < registry.sinks.GetCount(); ++i)
		{
			if (registry.sinks[i] == pSink)
			{
				registry.sinks.RemoveAt(i);
				return;
			}
		}
	}

	void Terminal::Print(LogLevel level, std::string_view titleName, std::string_view message)
	{
		const auto utcNow = std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now());
		const auto local = utcNow + CachedUtcOffset();

		SinkRegistry& registry = Registry();
		std::lock_guard lock(registry.mutex);

		if (UseColor())
		{
			constexpr const c8* kOrange = "\033[38;5;208m";
			constexpr const c8* kReset = "\033[0m";

			std::println("[{:%H:%M:%S}][{}{}{}][{}{}{}]: {}", local,
				kOrange, titleName, kReset,
				ColorFor(level), NameFor(level), kReset, message);
		}
		else
		{
			std::println("[{:%H:%M:%S}][{}][{}]: {}", local, titleName, NameFor(level), message);
		}

		for (ILogSink* pSink : registry.sinks)
			pSink->OnMessage(level, titleName, message);
	}

	void Terminal::Abort(std::string_view titleName, std::string_view message)
	{
#if defined(HORIZON_WINDOWS)
		if (IsDebuggerPresent())
			__debugbreak();
#endif
		std::abort();
	}
}