#include "Win32FileHelpers.h"

namespace Horizon::PAL
{
	DWORD Win32FileHelpers::ToSharePolicy(FileOperationSharePolicy sharePolicy)
	{
		if (sharePolicy == FileOperationSharePolicy::Exclusive)
			return 0;

		DWORD flags = 0;
		if (HasFlag(sharePolicy, FileOperationSharePolicy::SharedWrite))
			flags |= FILE_SHARE_WRITE;

		if (HasFlag(sharePolicy, FileOperationSharePolicy::SharedRead))
			flags |= FILE_SHARE_READ;

		if (HasFlag(sharePolicy, FileOperationSharePolicy::SharedDeleteRename))
			flags |= FILE_SHARE_DELETE;

		return flags;
	}

	DWORD Win32FileHelpers::ToAccessPolicy(FileOperationAccessPolicy accessPolicy)
	{
		if (accessPolicy == FileOperationAccessPolicy::None)
			return 0;

		DWORD flags = 0;
		if (HasFlag(accessPolicy, FileOperationAccessPolicy::Read))
			flags |= GENERIC_READ;

		if (HasFlag(accessPolicy, FileOperationAccessPolicy::Write))
			flags |= GENERIC_WRITE;

		return flags;
	}

	DWORD Win32FileHelpers::ToAsyncPolicy(b8 asyncOp)
	{
		if (asyncOp)
			return FILE_FLAG_OVERLAPPED | FILE_FLAG_SEQUENTIAL_SCAN;

		return FILE_ATTRIBUTE_NORMAL;
	}
}