#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>
#include <Runtime/PAL/File/FileOperationPolicies.h>

#include <Windows.h>

namespace Horizon::PAL
{
	struct Win32FileHelpers
	{
		static DWORD ToSharePolicy(FileOperationSharePolicy sharePolicy);
		static DWORD ToAccessPolicy(FileOperationAccessPolicy accessPolicy);
		static DWORD ToAsyncPolicy(b8 asyncOp);
	};
}