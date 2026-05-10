#pragma once

#include <string>

namespace Horizon
{
	class EngineReport
	{
	public:
		EngineReport() = default;
		EngineReport(std::string type, std::string message) : systemType(type), reportMessage(message) {}

		operator bool() const
		{
			bool success = reportMessage.empty();
			if (!success)
				Log::Terminal(LogType::Error, systemType, reportMessage);

			return success;
		}

		std::string systemType;
		std::string reportMessage;
	};
}