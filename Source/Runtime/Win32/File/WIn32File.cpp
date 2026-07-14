#include <Runtime/PAL/File/File.h>

#include <Runtime/Win32/Helpers/Win32FileHelpers.h>
#include <Runtime/Win32/Helpers/Win32ErrorHelpers.h>

#include <Windows.h>

namespace Horizon::PAL
{
	FileAccessRequest File::RequestAccess(const std::filesystem::path& newPath, FileOperationAccessPolicy accessPol, FileOperationSharePolicy sharePol, b8 asyncOp)
	{
		DWORD access = PAL::Win32FileHelpers::ToAccessPolicy(accessPol);
		DWORD share = PAL::Win32FileHelpers::ToSharePolicy(sharePol);
		DWORD async = PAL::Win32FileHelpers::ToAsyncPolicy(asyncOp);

		HANDLE fileHandle = CreateFileA(newPath.string().data(), access, share, NULL, OPEN_EXISTING, async, NULL);
		if (fileHandle == NULL || fileHandle == INVALID_HANDLE_VALUE)
		{
			std::string err = Win32ErrorHelpers::GetLastErrorString(GetLastError());
			Terminal::Error("FileAccessRequest", "{}", err);
			return FileAccessRequest();
		}

		return FileAccessRequest(FileAccessHandle::Generate(u64(fileHandle)), accessPol, sharePol, asyncOp);
	}

	void File::ReleaseAccess(FileAccessRequest handle)
	{
		if (!handle.m_handle.IsValid())
		{
			Terminal::Error("FileAccessRequest", "Invalid handle passed to ReleaseAccess");
			return;
		}

		HANDLE fileHandle = (HANDLE)handle.m_handle.index;
		if (!CloseHandle(fileHandle))
		{
			Terminal::Error("FileAccessRequest", "{}",
				Win32ErrorHelpers::GetLastErrorString(GetLastError()));
		}

		handle.m_handle = {};
	}

	b8 File::Create(const std::filesystem::path& newPath)
	{
		HANDLE fileHandle = CreateFileA(newPath.string().data(), GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (fileHandle == NULL || fileHandle == INVALID_HANDLE_VALUE)
		{
			std::string err = Win32ErrorHelpers::GetLastErrorString(GetLastError());
			Terminal::Error("File::Create", "{}", err);
			return false;
		}

		if (CloseHandle(fileHandle) == 0)
			return false;

		return true;
	}

	b8 File::Delete(const std::filesystem::path& newPath)
	{
		if (!DeleteFileA(newPath.string().data()))
		{
			std::string err = Win32ErrorHelpers::GetLastErrorString(GetLastError());
			Terminal::Error("File::Delete", "{}", err);
			return false;
		}

		return true;
	}

	b8 File::WriteString(FileAccessRequest fileAccess, const std::string& content, usize offset)
	{
		return false;
	}
}