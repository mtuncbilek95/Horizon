#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>
#include <Runtime/PAL/File/FileView.h>
#include <Runtime/PAL/File/FileAccessRequest.h>
#include <Runtime/Containers/List.h>

#include <string>

namespace Horizon::PAL
{
	struct H_EXPORT File final
	{
		static FileAccessRequest RequestAccess(const std::string& newPath, FileOperationAccessPolicy accessPol, FileOperationSharePolicy sharePol, b8 asyncOp = false);
		static void ReleaseAccess(FileAccessRequest handle);

		static FileView OpenMap(FileAccessRequest fileAccess, usize offset = 0, usize size = 0);
		static b8 CloseMap(FileView& view);

		static b8 Create(const std::string& newPath);
		static b8 Delete(const std::string& newPath);
		static b8 Exists(const std::string& newPath);
		static b8 WriteString(FileAccessRequest fileAccess, const std::string& content, usize offset = 0);
		static b8 WriteMemory(FileAccessRequest fileAccess, const List<u8>& memory, usize offset = 0);
		static b8 ReadMemory(FileAccessRequest fileAccess, List<u8>& memory, usize startPoint = 0, usize endPoint = 0);
		static b8 ReadString(FileAccessRequest fileAccess, std::string& outString, usize startPoint = 0, usize endPoint = 0);

		static b8 RenameWithLock(FileAccessRequest fileAccess, const std::string oldPath, const std::string newPath);
		static b8 Rename(const std::string oldPath, const std::string newPath);
	};
}