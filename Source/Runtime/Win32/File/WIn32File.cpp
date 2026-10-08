#include <Runtime/PAL/File/File.h>

#include <Runtime/Log/Terminal.h>

#include <Runtime/Win32/Helpers/Win32FileHelpers.h>
#include <Runtime/Win32/Helpers/Win32ErrorHelpers.h>

#include <Windows.h>

namespace Horizon::PAL
{
	FileAccessRequest File::RequestAccess(const std::string& newPath, FileOperationAccessPolicy accessPol, FileOperationSharePolicy sharePol, b8 asyncOp)
	{
		DWORD access = PAL::Win32FileHelpers::ToAccessPolicy(accessPol);
		DWORD share = PAL::Win32FileHelpers::ToSharePolicy(sharePol);
		DWORD async = PAL::Win32FileHelpers::ToAsyncPolicy(asyncOp);

		HANDLE fileHandle = CreateFileA(newPath.data(), access, share, NULL, OPEN_EXISTING, async, NULL);
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

	FileView File::OpenMap(FileAccessRequest fileAccess, usize offset, usize size)
	{
		if (!fileAccess.m_handle.IsValid())
		{
			Terminal::Error("File::OpenMap", "Invalid file access handle");
			return FileView();
		}

		if (((u8)fileAccess.GetAccessPolicy() & (u8)FileOperationAccessPolicy::Read) == 0)
		{
			Terminal::Error("File::OpenMap", "File was not opened with Read access");
			return FileView();
		}

		HANDLE fileHandle = (HANDLE)fileAccess.m_handle.index;

		LARGE_INTEGER fileSize = {};
		if (!GetFileSizeEx(fileHandle, &fileSize))
		{
			Terminal::Error("File::OpenMap", "{}", Win32ErrorHelpers::GetLastErrorString(GetLastError()));
			return FileView();
		}

		usize fileEnd = (usize)fileSize.QuadPart;
		usize mapEnd = (size == 0) ? fileEnd : offset + size;

		if (offset > mapEnd || mapEnd > fileEnd)
		{
			Terminal::Error("File::OpenMap", "Range [{}, {}) out of bounds, file size is {}",
				offset, mapEnd, fileEnd);
			return FileView();
		}

		usize mapSize = mapEnd - offset;
		if (mapSize == 0)
		{
			Terminal::Error("File::OpenMap", "Cannot map an empty range at {}", offset);
			return FileView();
		}

		SYSTEM_INFO sysInfo = {};
		GetSystemInfo(&sysInfo);

		usize granularity = (usize)sysInfo.dwAllocationGranularity;
		usize baseOffset = offset - (offset % granularity);
		usize delta = offset - baseOffset;
		usize viewSize = mapSize + delta;

		HANDLE mapping = CreateFileMappingA(fileHandle, NULL, PAGE_READONLY, 0, 0, NULL);
		if (mapping == NULL)
		{
			Terminal::Error("File::OpenMap", "{}", Win32ErrorHelpers::GetLastErrorString(GetLastError()));
			return FileView();
		}

		ULARGE_INTEGER pos = {};
		pos.QuadPart = (ULONGLONG)baseOffset;

		void* pBase = MapViewOfFile(mapping, FILE_MAP_READ, pos.HighPart, pos.LowPart, viewSize);
		DWORD mapError = GetLastError();

		if (!CloseHandle(mapping))
			Terminal::Error("File::OpenMap", "{}", Win32ErrorHelpers::GetLastErrorString(GetLastError()));

		if (pBase == NULL)
		{
			Terminal::Error("File::OpenMap", "{}", Win32ErrorHelpers::GetLastErrorString(mapError));
			return FileView();
		}

		return FileView(pBase, (const u8*)pBase + delta, mapSize);
	}

	b8 File::CloseMap(FileView& view)
	{
		if (!view.IsValid())
		{
			Terminal::Error("File::CloseMap", "Invalid view passed to CloseMap");
			return false;
		}

		b8 result = UnmapViewOfFile(view.m_base);
		if (!result)
			Terminal::Error("File::CloseMap", "{}", Win32ErrorHelpers::GetLastErrorString(GetLastError()));

		view.Release();
		return result;
	}

	b8 File::Create(const std::string& newPath)
	{
		HANDLE fileHandle = CreateFileA(newPath.data(), GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
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

	b8 File::Delete(const std::string& newPath)
	{
		if (!DeleteFileA(newPath.data()))
		{
			std::string err = Win32ErrorHelpers::GetLastErrorString(GetLastError());
			Terminal::Error("File::Delete", "{}", err);
			return false;
		}

		return true;
	}

	b8 File::Exists(const std::string& newPath)
	{
		DWORD dwAttrib = GetFileAttributes(newPath.data());
		return (dwAttrib != INVALID_FILE_ATTRIBUTES && !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
	}

	b8 File::WriteString(FileAccessRequest fileAccess, const std::string& content, usize offset)
	{
		List<u8> bytes(content.size());
		std::memcpy(bytes.GetData(), content.data(), content.size());
		return WriteMemory(fileAccess, bytes, offset);
	}


	b8 File::WriteMemory(FileAccessRequest fileAccess, const List<u8>& memory, usize offset)
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

	b8 File::Truncate(FileAccessRequest fileAccess, usize size)
	{
		if (!fileAccess.m_handle.IsValid())
		{
			Terminal::Error("File::Truncate", "Invalid file access handle");
			return false;
		}

		if (((u8)fileAccess.GetAccessPolicy() & (u8)FileOperationAccessPolicy::Write) == 0)
		{
			Terminal::Error("File::Truncate", "File was not opened with Write access");
			return false;
		}

		HANDLE fileHandle = (HANDLE)fileAccess.m_handle.index;

		LARGE_INTEGER pos = {};
		pos.QuadPart = (LONGLONG)size;

		if (!SetFilePointerEx(fileHandle, pos, NULL, FILE_BEGIN))
		{
			Terminal::Error("File::Truncate", "{}", Win32ErrorHelpers::GetLastErrorString(GetLastError()));
			return false;
		}

		if (!SetEndOfFile(fileHandle))
		{
			Terminal::Error("File::Truncate", "{}", Win32ErrorHelpers::GetLastErrorString(GetLastError()));
			return false;
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

	b8 File::ReadString(FileAccessRequest fileAccess, std::string& outString, usize startPoint, usize endPoint)
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

		outString.clear();
		if (readSize == 0)
			return true;

		LARGE_INTEGER pos = {};
		pos.QuadPart = (LONGLONG)startPoint;
		if (!SetFilePointerEx(fileHandle, pos, NULL, FILE_BEGIN))
		{
			Terminal::Error("File::ReadMemory", "{}", Win32ErrorHelpers::GetLastErrorString(GetLastError()));
			return false;
		}

		outString.resize(readSize);

		usize totalRead = 0;
		while (totalRead < readSize)
		{
			usize remaining = readSize - totalRead;
			DWORD chunk = (DWORD)(remaining > MAXDWORD ? MAXDWORD : remaining);

			DWORD bytesRead = 0;
			if (!ReadFile(fileHandle, outString.data() + totalRead, chunk, &bytesRead, NULL))
			{
				Terminal::Error("File::ReadMemory", "{}", Win32ErrorHelpers::GetLastErrorString(GetLastError()));
				outString.clear();
				return false;
			}

			if (bytesRead == 0)
			{
				Terminal::Error("File::ReadMemory", "Unexpected EOF at {} of {}", totalRead, readSize);
				outString.clear();
				return false;
			}

			totalRead += bytesRead;
		}

		return true;
	}


	b8 File::RenameWithLock(FileAccessRequest fileAccess, const std::string oldPath, const std::string newPath)
	{
		if (!fileAccess.m_handle.IsValid())
		{
			Terminal::Error("File::Rename", "Invalid file access handle");
			return false;
		}

		if (((u8)fileAccess.GetAccessPolicy() & (u8)FileOperationAccessPolicy::Rename) == 0)
		{
			Terminal::Error("File::Rename", "File was not opened with Rename access");
			return false;
		}

		const i32 wideCount = MultiByteToWideChar(CP_UTF8, 0, newPath.data(), (i32)newPath.size(), nullptr, 0);

		if (wideCount <= 0)
		{
			Terminal::Error("File::Rename", "Cannot widen path: {}", newPath);
			return false;
		}

		const usize bufferSize = sizeof(FILE_RENAME_INFO) + (usize)wideCount * sizeof(WCHAR);
		List<u8> buffer(bufferSize);

		FILE_RENAME_INFO* pInfo = reinterpret_cast<FILE_RENAME_INFO*>(buffer.GetData());
		pInfo->ReplaceIfExists = FALSE;
		pInfo->RootDirectory = NULL;
		pInfo->FileNameLength = (DWORD)(wideCount * sizeof(WCHAR));

		MultiByteToWideChar(CP_UTF8, 0, newPath.data(), (i32)newPath.size(), pInfo->FileName, wideCount);

		HANDLE fileHandle = (HANDLE)fileAccess.m_handle.index;
		b8 result = SetFileInformationByHandle(fileHandle, FileRenameInfo, pInfo, (DWORD)bufferSize);

		if (!result)
			Terminal::Error("File::Rename", "Rename failed: {}", newPath);

		return result;
	}

	b8 File::Rename(const std::string oldPath, const std::string newPath)
	{
		return MoveFile(oldPath.data(), newPath.data());
	}

	b8 File::Copy(const std::string& sourcePath, const std::string& targetFolder)
	{
		const std::string source = StringOps::NormalizePath(sourcePath);
		const usize slash = source.rfind('/');
		const std::string fileName = slash == std::string::npos ? source : source.substr(slash + 1);
		const std::string target = StringOps::NormalizePath(targetFolder) + "/" + fileName;

		if (!CopyFile(source.data(), target.data(), FALSE))
		{
			Terminal::Error("File", "{} could not be copied to {}", source, target);
			return false;
		}

		return true;
	}
}