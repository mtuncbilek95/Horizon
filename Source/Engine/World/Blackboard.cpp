#include "Blackboard.h"

#include <Runtime/Containers/ScopedLock.h>
#include <Runtime/Containers/StringHelper.h>
#include <Runtime/Log/Terminal.h>

namespace Horizon::Engine
{
	Blackboard::~Blackboard()
	{
		Reset();
	}

	void Blackboard::Reset()
	{
		ScopedLock<PAL::Mutex> lock(m_lock);

		while (!m_entries.IsEmpty())
			m_entries.PopBack();

		m_lookup.clear();
	}

	void* Blackboard::Replace(Reflect::TypeHandle handle)
	{
		ScopedLock<PAL::Mutex> lock(m_lock);

		auto it = m_lookup.find(handle);

		if (it == m_lookup.end())
			return nullptr;

		Terminal::Error(StringHelper::GetTypeName(this), "{} type has already been added this frame. Returning the existing entry.", handle.Index());
		return m_entries[it->second]->GetData();
	}

	void Blackboard::Insert(Reflect::TypeHandle handle, UniquePtr<Entry> entry)
	{
		ScopedLock<PAL::Mutex> lock(m_lock);

		m_lookup[handle] = m_entries.GetCount();
		m_entries.PushBack(std::move(entry));
	}

	void* Blackboard::Find(Reflect::TypeHandle handle, b8 logMissing) const
	{
		ScopedLock<PAL::Mutex> lock(m_lock);

		auto it = m_lookup.find(handle);

		if (it == m_lookup.end())
		{
			if (logMissing)
				Terminal::Error(StringHelper::GetTypeName(this), "{} type has no entry. Check which system is supposed to write it.", handle.Index());

			return nullptr;
		}

		return m_entries[it->second]->GetData();
	}
}