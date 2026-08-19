#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>
#include <Runtime/Containers/List.h>
#include <Runtime/PAL/Watcher/WatcherAction.h>
#include <Runtime/PAL/Watcher/WatcherEntryKind.h>

#include <string>
#include <string_view>

namespace Horizon::PAL
{
	using WatcherHandle = void*;

	class H_EXPORT DirectoryWatcher
	{
	public:
		struct Event
		{
			static constexpr u32 NoExtension = 0xFFFFFF;

			WatcherAction action;
			WatcherEntryKind kind;

			std::string absolutePath;
			std::string relativePath;
			u32 nameOffset = 0;
			u32 extensionOffset = NoExtension;

			std::string oldRelativePath;
			u32 oldNameOffset = 0;

			std::string_view GetParent() const;
			std::string_view GetName() const;
			std::string_view GetExtension() const;
			std::string_view GetOldParent() const;
			std::string_view GetOldName() const;

			b8 IsRootLevel() const { return nameOffset == 0; }
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