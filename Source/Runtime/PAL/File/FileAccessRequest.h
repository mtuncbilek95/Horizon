#pragma once

#include <Runtime/PAL/File/FileOperationPolicies.h>

namespace Horizon::PAL
{
	struct FileAccessTag {};
	using FileAccessHandle = Handle<FileAccessTag>;

	class FileAccessRequest final
	{
		friend struct File;
	public:
		FileAccessRequest();
		FileAccessRequest(FileAccessHandle hndl, FileOperationAccessPolicy accessPolicy,
			FileOperationSharePolicy sharePolicy, b8 asyncOp);
		~FileAccessRequest() = default;

		b8 IsValid() const { return m_handle.IsValid(); }
		FileAccessHandle GetAccessHandle() const { return m_handle; }
		FileOperationAccessPolicy GetAccessPolicy() const { return m_access; }
		FileOperationSharePolicy GetSharePolicy() const { return m_share; }

	private:
		void Release();

	private:
		FileAccessHandle m_handle;
		FileOperationAccessPolicy m_access;
		FileOperationSharePolicy m_share;
		b8 m_asyncOp;
	};
}