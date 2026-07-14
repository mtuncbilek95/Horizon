#include "FileAccessRequest.h"

namespace Horizon::PAL
{
	FileAccessRequest::FileAccessRequest() : m_handle(FileAccessHandle()), m_access(FileOperationAccessPolicy::None),
		m_share(FileOperationSharePolicy::Exclusive), m_asyncOp(false)
	{
	}

	FileAccessRequest::FileAccessRequest(FileAccessHandle hndl, FileOperationAccessPolicy accessPolicy, FileOperationSharePolicy sharePolicy, b8 asyncOp) : m_handle(hndl),
		m_access(accessPolicy), m_share(sharePolicy), m_asyncOp(asyncOp)
	{
	}

	void FileAccessRequest::Release()
	{
		m_handle = FileAccessHandle();
	}
}