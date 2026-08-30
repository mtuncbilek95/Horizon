#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>
#include <Runtime/Containers/List.h>
#include <Runtime/PAL/Watcher/WatcherAction.h>
#include <Runtime/PAL/Watcher/WatcherEntryKind.h>

#include <functional>
#include <string>
#include <string_view>
#include <utility>

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

		using EventHandler = std::function<void(const Event&)>;
		using SubscriptionId = u32;

		static constexpr SubscriptionId InvalidSubscription = 0;

		DirectoryWatcher() = default;
		DirectoryWatcher(const std::string& rootPath, b8 recursive = true);
		~DirectoryWatcher();

		DirectoryWatcher(const DirectoryWatcher&) = delete;
		DirectoryWatcher& operator=(const DirectoryWatcher&) = delete;

		DirectoryWatcher(DirectoryWatcher&& other) noexcept;
		DirectoryWatcher& operator=(DirectoryWatcher&& other) noexcept;

		SubscriptionId Subscribe(WatcherAction action, EventHandler handler);
		void Unsubscribe(SubscriptionId id);
		void ClearSubscriptions();

		SubscriptionId OnAdded(EventHandler handler) { return Subscribe(WatcherAction::Added, std::move(handler)); }
		SubscriptionId OnRemoved(EventHandler handler) { return Subscribe(WatcherAction::Removed, std::move(handler)); }
		SubscriptionId OnModified(EventHandler handler) { return Subscribe(WatcherAction::Modified, std::move(handler)); }
		SubscriptionId OnRenamed(EventHandler handler) { return Subscribe(WatcherAction::Renamed, std::move(handler)); }
		SubscriptionId OnOverflow(EventHandler handler) { return Subscribe(WatcherAction::Overflow, std::move(handler)); }

		b8 Dispatch();
		b8 Poll(List<Event>& outEvents);

		b8 IsValid() const { return m_handle != nullptr; }
		const std::string& GetRootPath() const { return m_rootPath; }

	private:
		struct Subscription
		{
			SubscriptionId id = InvalidSubscription;
			WatcherAction action = WatcherAction::Added;
			EventHandler handler;
		};

		WatcherHandle m_handle = nullptr;
		std::string m_rootPath;

		List<Subscription> m_subscriptions;
		SubscriptionId m_nextSubscription = 1;
		List<Event> m_events;
	};
}