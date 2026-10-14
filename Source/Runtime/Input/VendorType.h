#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>

namespace Horizon::Input
{
	enum class VendorType : u16
	{
		Unknown = 0,
		Sony = 0x054C,
		Microsoft = 0x045E,
		Nintendo = 0x057E,
		Logitech = 0x046D
	};
}