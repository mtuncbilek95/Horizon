#include <Runtime/PAL/File/Directory.h>
#include <Runtime/Log/Terminal.h>
#include <Runtime/Win32/Helpers/Win32ErrorHelpers.h>

#include <Windows.h>

namespace Horizon::PAL
{
	b8 Directory::Create(const std::string& path)
	{
		b8 result = CreateDirectoryA(path.data(), NULL);
		return result;
	}

	b8 Directory::Delete(const std::string& path)
	{
		b8 result = RemoveDirectoryA(path.data());
		return result;
	}

	b8 Directory::Exists(const std::string& path)
	{
		DWORD dwAttrib = GetFileAttributes(path.data());
		return (dwAttrib != INVALID_FILE_ATTRIBUTES && (dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
	}

	List<Directory::Entry> Directory::Iterate(const std::string& path)
	{
		List<Directory::Entry> result;

		std::string searchPath = path + "\\*";
		WIN32_FIND_DATA findData;

		HANDLE hFind = FindFirstFile(searchPath.data(), &findData);

		if (hFind == INVALID_HANDLE_VALUE)
			return result;

		do 
		{
			const std::string name = findData.cFileName;
			if (name == "." || name == "..")
				continue;

			Entry entry;
			entry.name = name;
			entry.fullPath = path + "/" + name;
			entry.isDirectory = (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
			result.PushBack(entry);

		} while (FindNextFile(hFind, &findData));

		FindClose(hFind);
		return result;
	}

	b8 Directory::Rename(const std::string oldPath, const std::string newPath)
	{
		return MoveFile(oldPath.data(), newPath.data());
	}
}