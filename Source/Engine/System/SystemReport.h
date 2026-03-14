#pragma once

namespace Horizon
{
	/*
	* System report is a struct that returns false if it has a message, and true if it doesn't. The 
	* message is used to report any errors or warnings that occurred during the system's initialization, 
	* and can be used by the engine to log or display the message to the user.
	*/
	struct SystemReport
	{
		SystemReport() = default;
		SystemReport(std::string_view type, std::string_view message) : systemType(type), reportMessage(message) {}

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