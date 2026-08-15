#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>
#include <Runtime/Containers/List.h>
#include <Runtime/PAL/Watcher/WatcherAction.h>

#include <string>

namespace Horizon::PAL
{
	using WatcherHandle = void*;

	class H_EXPORT DirectoryWatcher
	{
	public:
		struct Event
		{
			WatcherAction action;
			std::string path;
			std::string relativePath;
			std::string oldPath;
			std::string oldRelativePath;
			b8 isDirectory;
		};

		DirectoryWatcher() = default;
		DirectoryWatcher(const std::string& rootPath, b8 recursive = true);
		~DirectoryWatcher();

		DirectoryWatcher(const DirectoryWatcher&) = delete;
		DirectoryWatcher& operator=(const DirectoryWatcher&) = delete;

		DirectoryWatcher(DirectoryWatcher&& other) noexcept;
		DirectoryWatcher& operator=(DirectoryWatcher&& other) noexcept;

		b8 Poll(List<Event>& outEvents);
		b8 IsValid() const { return m_handle != nullptr; }

		const std::string& GetRootPath() const { return m_rootPath; }

	private:
		WatcherHandle m_handle = nullptr;
		std::string m_rootPath;
	};
}