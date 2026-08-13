#pragma once

#include <Runtime/Definitions/BitwiseOperators.h>
#include <Runtime/Definitions/PrimitiveDefinitions.h>

namespace Horizon::PAL
{
	enum class FileOperationSharePolicy : u8
	{
		Exclusive = 0,
		SharedRead = 1 << 0,
		SharedWrite = 1 << 1,
		SharedDeleteRename = 1 << 2
	};

	enum class FileOperationAccessPolicy : u8 
	{
		None = 0,
		Read = 1 << 0,
		Write = 1 << 1,
		Rename = 1 << 2
	};
}