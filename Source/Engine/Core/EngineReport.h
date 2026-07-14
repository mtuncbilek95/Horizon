#pragma once

#include <string>
#include <string_view>

namespace Horizon
{
	struct EngineReport
	{
		EngineReport() = default;
		EngineReport(std::string_view message) : m_message(message) {}

		explicit operator bool() const { return !m_message.empty(); }
		std::string_view GetMessage() const { return m_message; }

	private:
		std::string m_message;
	};
}