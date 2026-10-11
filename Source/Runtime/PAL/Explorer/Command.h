#pragma once

#include <string>

namespace Horizon::PAL
{
	struct RUNTIME_API Command
	{
		static void ExecuteInTerminal(const std::string& commandLine);
	};
}