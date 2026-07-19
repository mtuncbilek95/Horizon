#pragma once

#include <Windows.h>
#include <string>

namespace Horizon::PAL
{
	struct Win32ErrorHelpers
	{
		static std::string GetLastErrorString(DWORD errCode);
	};
}