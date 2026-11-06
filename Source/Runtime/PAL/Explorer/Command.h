#pragma once

#include <string>

namespace Horizon::PAL
{
	struct Command
	{
		static void ExecuteInTerminal(const std::string& commandLine);
	};
}