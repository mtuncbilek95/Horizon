#include "LooseSourceFile.h"

#include <Runtime/Containers/StringOps.h>
#include <Runtime/Log/Terminal.h>

namespace Horizon::Engine
{
	LooseSourceFile::LooseSourceFile(const std::string& name) : m_name(name)
	{
	}

	const AssetEntry* LooseSourceFile::Find(const Guid& id) const
	{
		auto it = m_entries.find(id);

		if (it == m_entries.end())
			return nullptr;

		return &it->second;
	}

	b8 LooseSourceFile::Register(const AssetEntry& entry)
	{
		if (entry.assetId == Guid())
		{
			Terminal::Error(StringOps::GetName(this), "{} cannot be registered to {} with an empty id", entry.cookedPath, m_name);
			return false;
		}

		if (entry.cookedPath.empty())
		{
			Terminal::Error(StringOps::GetName(this), "{} cannot be registered to {} without a cooked path", entry.assetId.ToString(), m_name);
			return false;
		}

		m_entries[entry.assetId] = entry;

		return true;
	}

	b8 LooseSourceFile::Unregister(const Guid& id)
	{
		auto it = m_entries.find(id);

		if (it == m_entries.end())
		{
			Terminal::Warn(StringOps::GetName(this), "{} is not registered in {}", id.ToString(), m_name);
			return false;
		}

		m_entries.erase(it);

		return true;
	}

	void LooseSourceFile::Clear()
	{
		m_entries.clear();
	}
}