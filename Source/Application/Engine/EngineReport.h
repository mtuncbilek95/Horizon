#pragma once

#include <string_view>

namespace Horizon
{
	class EngineReport
	{
	public:
		EngineReport() = default;
		EngineReport(std::string_view type, std::string_view message) : systemType(type), reportMessage(message) {}

		operator bool() const
		{
			bool success = reportMessage.empty();
			if (!success)
				Log::Terminal(LogType::Error, systemType, reportMessage);

			return success;
		}

		std::string_view systemType;
		std::string_view reportMessage;
	};
}