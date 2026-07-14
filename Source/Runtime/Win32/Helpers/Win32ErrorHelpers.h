#pragma once

#include <Windows.h>

namespace Horizon::PAL
{
	struct Win32ErrorHelpers
	{
		static std::string GetLastErrorString(DWORD errCode);
	};
}