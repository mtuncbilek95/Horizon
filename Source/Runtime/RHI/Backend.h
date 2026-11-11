#pragma once

#include <Runtime/RTTR/Reflection.h>

namespace Horizon::RHI
{
	HENUM();
	enum class Backend : u8
	{
		DX12,
		Vulkan
	};
}