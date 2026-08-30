#include "DirectoryWatcher.h"

#include <Runtime/Log/Terminal.h>

#include <utility>

namespace Horizon::PAL
{
	std::string_view DirectoryWatcher::Event::GetParent() const
	{
		return nameOffset == 0 ? std::string_view() : std::string_view(relativePath.data(), nameOffset - 1);
	}

	std::string_view DirectoryWatcher::Event::GetName() const
	{
		return std::string_view(relativePath).substr(nameOffset);
	}

	std::string_view DirectoryWatcher::Event::GetExtension() const
	{
		return extensionOffset == NoExtension ? std::string_view() : std::string_view(relativePath).substr(extensionOffset);
	}

	std::string_view DirectoryWatcher::Event::GetOldParent() const
	{
		return oldNameOffset == 0 ? std::string_view() : std::string_view(oldRelativePath.data(), oldNameOffset - 1);
	}

	std::string_view DirectoryWatcher::Event::GetOldName() const
	{
		return std::string_view(oldRelativePath).substr(oldNameOffset);
	}

	DirectoryWatcher::SubscriptionId DirectoryWatcher::Subscribe(WatcherAction action, EventHandler handler)
	{
		if (handler == nullptr)
		{
			Terminal::Warn("DirectoryWatcher", "{} received an empty handler", m_rootPath);
			return InvalidSubscription;
		}

		Subscription subscription;
		subscription.id = m_nextSubscription++;
		subscription.action = action;
		subscription.handler = std::move(handler);

		const SubscriptionId id = subscription.id;
		m_subscriptions.PushBack(std::move(subscription));

		return id;
	}

	void DirectoryWatcher::Unsubscribe(SubscriptionId id)
	{
		for (usize i = 0; i < m_subscriptions.GetCount(); ++i)
		{
			if (m_subscriptions[i].id != id)
				continue;

			m_subscriptions.RemoveAt(i);
			return;
		}

		Terminal::Warn("DirectoryWatcher", "{} subscription is not registered", id);
	}

	void DirectoryWatcher::ClearSubscriptions()
	{
		m_subscriptions.Clear();
	}

	b8 DirectoryWatcher::Dispatch()
	{
		m_events.Clear();

		const b8 healthy = Poll(m_events);

		for (const Event& event : m_events)
		{
			for (usize i = 0; i < m_subscriptions.GetCount(); ++i)
			{
				const Subscription& subscription = m_subscriptions[i];

				if (subscription.action != event.action)
					continue;

				subscription.handler(event);
			}
		}

		return healthy;
	}
}