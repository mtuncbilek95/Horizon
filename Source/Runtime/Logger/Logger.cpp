#include "Logger.h"

#include <chrono>
#include <cstdlib>
#include <ctime>
#include <print>
#include <string>

namespace Horizon
{
	static std::string_view TypeColor(LogType type)
	{
		switch (type)
		{
		case LogType::Info: return "\033[37m";
		case LogType::Debug: return "\033[36m";
		case LogType::Success: return "\033[32m";
		case LogType::Warning: return "\033[33m";
		case LogType::Error: return "\033[91m";
		case LogType::Fatal: return "\033[95m";
		default: return "\033[0m";
		}
	}

	static std::string_view TypeLabel(LogType type)
	{
		switch (type)
		{
		case LogType::Info: return "Info";
		case LogType::Debug: return "Debug";
		case LogType::Success: return "Success";
		case LogType::Warning: return "Warning";
		case LogType::Error: return "Error";
		case LogType::Fatal: return "Fatal";
		default: return "Unknown";
		}
	}

	static std::string Timestamp()
	{
		auto now = std::chrono::system_clock::now();
		auto t = std::chrono::system_clock::to_time_t(now);
		auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

		std::tm tm{};
#ifdef _WIN32
		localtime_s(&tm, &t);
#else
		localtime_r(&t, &tm);
#endif

		return std::format("{:02}:{:02}:{:02}.{:03}",
			tm.tm_hour, tm.tm_min, tm.tm_sec, ms.count());
	}

	void Log::Print(LogType type, std::string_view category, std::string_view message)
	{
		constexpr std::string_view reset = "\033[0m";

		std::println("{}[{}][{}][{}] {}{}",
			TypeColor(type), Timestamp(), TypeLabel(type), category, message, reset);

		if (type == LogType::Fatal)
			std::abort();
	}
}
