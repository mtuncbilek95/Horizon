#include <Runtime/PAL/File/File.h>

#include <Runtime/Log/Terminal.h>

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
		List<u8> bytes(content.size());
		std::memcpy(bytes.GetData(), content.data(), content.size());
		return WriteMemory(fileAccess, bytes, offset);
	}


	b8 File::WriteMemory(FileAccessRequest fileAccess, const List<u8>& memory, usize offset /*= 0*/)
	{
		if (!fileAccess.m_handle.IsValid())
		{
			Terminal::Error("File::WriteMemory", "Invalid file access handle");
			return false;
		}

		if (((u8)fileAccess.GetAccessPolicy() & (u8)FileOperationAccessPolicy::Write) == 0)
		{
			Terminal::Error("File::WriteMemory", "File was not opened with Write access");
			return false;
		}

		HANDLE fileHandle = (HANDLE)fileAccess.m_handle.index;

		LARGE_INTEGER pos = {};
		pos.QuadPart = (LONGLONG)offset;
		if (!SetFilePointerEx(fileHandle, pos, NULL, FILE_BEGIN))
		{
			Terminal::Error("File::WriteMemory", "{}", Win32ErrorHelpers::GetLastErrorString(GetLastError()));
			return false;
		}

		usize totalWritten = 0;
		while (totalWritten < memory.GetCount())
		{
			usize remaining = memory.GetCount() - totalWritten;
			DWORD chunk = (DWORD)(remaining > MAXDWORD ? MAXDWORD : remaining);

			DWORD written = 0;
			if (!WriteFile(fileHandle, memory.GetData() + totalWritten, chunk, &written, NULL))
			{
				Terminal::Error("File::WriteMemory", "{}", Win32ErrorHelpers::GetLastErrorString(GetLastError()));
				return false;
			}

			if (written == 0)
			{
				Terminal::Error("File::WriteMemory", "Wrote 0 bytes at {} of {}", totalWritten, memory.GetCount());
				return false;
			}

			totalWritten += written;
		}

		return true;
	}

	b8 File::ReadMemory(FileAccessRequest fileAccess, List<u8>& memory, usize startPoint, usize endPoint)
	{
		if (!fileAccess.m_handle.IsValid())
		{
			Terminal::Error("File::ReadMemory", "Invalid file access handle");
			return false;
		}

		if (((u8)fileAccess.GetAccessPolicy() & (u8)FileOperationAccessPolicy::Read) == 0)
		{
			Terminal::Error("File::ReadMemory", "File was not opened with Read access");
			return false;
		}

		HANDLE fileHandle = (HANDLE)fileAccess.m_handle.index;

		LARGE_INTEGER fileSize = {};
		if (!GetFileSizeEx(fileHandle, &fileSize))
		{
			Terminal::Error("File::ReadMemory", "{}", Win32ErrorHelpers::GetLastErrorString(GetLastError()));
			return false;
		}

		usize fileEnd = (usize)fileSize.QuadPart;
		usize readEnd = (endPoint == 0) ? fileEnd : endPoint;

		if (startPoint > readEnd || readEnd > fileEnd)
		{
			Terminal::Error("File::ReadMemory", "Range [{}, {}) out of bounds, file size is {}",
				startPoint, readEnd, fileEnd);
			return false;
		}

		usize readSize = readEnd - startPoint;

		memory.Clear();
		if (readSize == 0)
			return true;

		LARGE_INTEGER pos = {};
		pos.QuadPart = (LONGLONG)startPoint;
		if (!SetFilePointerEx(fileHandle, pos, NULL, FILE_BEGIN))
		{
			Terminal::Error("File::ReadMemory", "{}", Win32ErrorHelpers::GetLastErrorString(GetLastError()));
			return false;
		}

		memory.Resize(readSize);

		usize totalRead = 0;
		while (totalRead < readSize)
		{
			usize remaining = readSize - totalRead;
			DWORD chunk = (DWORD)(remaining > MAXDWORD ? MAXDWORD : remaining);

			DWORD bytesRead = 0;
			if (!ReadFile(fileHandle, memory.GetData() + totalRead, chunk, &bytesRead, NULL))
			{
				Terminal::Error("File::ReadMemory", "{}", Win32ErrorHelpers::GetLastErrorString(GetLastError()));
				memory.Clear();
				return false;
			}

			if (bytesRead == 0)
			{
				Terminal::Error("File::ReadMemory", "Unexpected EOF at {} of {}", totalRead, readSize);
				memory.Clear();
				return false;
			}

			totalRead += bytesRead;
		}

		return true;
	}
}