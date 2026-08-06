#pragma once

#include <Runtime/Containers/List.h>
#include <functional>

namespace Horizon
{
	template<typename T>
	class Selection
	{
	public:
		void Set(const T& item);
		void Add(const T& item);
		void Remove(const T& item);
		void Clear();

		b8 Contains(const T& item) const;
		b8 IsEmpty() const { return m_items.IsEmpty(); }

		const List<T>& Items() const { return m_items; }
		const T& Primary() const { return m_items.Back(); }

		void Subscribe(std::function<void()> callback) { m_listeners.PushBack(std::move(callback)); }

	private:
		void RaiseChanged();

	private:
		List<T> m_items;
		List<std::function<void()>> m_listeners;
	};
}