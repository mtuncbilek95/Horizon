#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>
#include <Runtime/PAL/File/FileAccessRequest.h>

#include <filesystem>
#include <string>

namespace Horizon::PAL
{
	struct File final
	{
		static FileAccessRequest RequestAccess(const std::filesystem::path& newPath, FileOperationAccessPolicy accessPol, FileOperationSharePolicy sharePol, b8 asyncOp = false);
		static void ReleaseAccess(FileAccessRequest handle);

		static b8 Create(const std::filesystem::path& newPath);
		static b8 Delete(const std::filesystem::path& newPath);
		static b8 WriteString(FileAccessRequest fileAccess, const std::string& content, usize offset = 0);
		static b8 WriteMemory(FileAccessRequest fileAccess, const std::vector<u8>& memory, usize offset = 0);
		static b8 ReadMemory(FileAccessRequest fileAccess, std::vector<u8>& memory, usize startPoint = 0, usize endPoint = 0);
	};
}