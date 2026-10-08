#include <Runtime/PAL/File/Directory.h>
#include <Runtime/PAL/File/File.h>

#include <Runtime/Containers/StringOps.h>
#include <Runtime/Log/Terminal.h>
#include <Runtime/Win32/Helpers/Win32ErrorHelpers.h>

#include <Windows.h>

namespace Horizon::PAL
{
	b8 Directory::Create(const std::string& path)
	{
		if (path.empty() || Exists(path))
			return true;

		const std::string parent = StringOps::ParentPathOf(path);
		if (!parent.empty() && !Create(parent))
			return false;

		if (CreateDirectoryA(path.data(), NULL))
			return true;

		if (GetLastError() == ERROR_ALREADY_EXISTS)
			return Exists(path);

		Terminal::Error("Directory", "{} cannot be created: {}", path, Win32ErrorHelpers::GetLastErrorString(GetLastError()));
		return false;
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

	b8 Directory::Copy(const std::string& sourcePath, const std::string& targetFolder)
	{
		const std::string source = StringOps::NormalizePath(sourcePath);

		if (!Exists(source))
		{
			Terminal::Error("Directory", "{} does not exist, nothing to copy", source);
			return false;
		}

		const usize slash = source.find_last_of('/');
		const std::string name = slash == std::string::npos ? source : source.substr(slash + 1);
		const std::string target = StringOps::NormalizePath(targetFolder) + "/" + name;

		if (target == source || target.starts_with(source + "/"))
		{
			Terminal::Error("Directory", "{} cannot be copied into itself", source);
			return false;
		}

		if (!Create(target))
			return false;

		b8 result = true;

		for (const Entry& entry : Iterate(source))
		{
			if (entry.isDirectory)
			{
				if (!Copy(entry.fullPath, target))
					result = false;
			}
			else
			{
				if (!File::Copy(entry.fullPath, target))
					result = false;
			}
		}

		return result;
	}
}