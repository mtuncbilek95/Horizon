#include "NameId.h"

#include <Runtime/Containers/StringOps.h>
#include <Runtime/Log/Terminal.h>
#include <Runtime/PAL/Sync/Mutex.h>
#include <Runtime/Containers/ScopedLock.h>

#include <unordered_map>

namespace Horizon
{
	namespace
	{
		struct InternTable
		{
			std::unordered_map<u64, std::string> entries;
			PAL::Mutex mutex;
		};

		InternTable& Table()
		{
			static InternTable instance;
			return instance;
		}

		u64 Fnv1a64(std::string_view text)
		{
			u64 hash = 14695981039346656037ull;

			for (c8 value : text)
			{
				hash ^= u64(u8(value));
				hash *= 1099511628211ull;
			}

			return hash;
		}
	}

	NameId::NameId(std::string_view text)
	{
		if (text.empty())
			return;

		m_hash = Fnv1a64(text);

		InternTable& table = Table();
		ScopedLock lock(table.mutex);

		auto it = table.entries.find(m_hash);

		if (it == table.entries.end())
		{
			table.entries.emplace(m_hash, std::string(text));
			return;
		}

		if (it->second != text)
			Terminal::Error(StringOps::GetName(this), "Hash collision between '{}' and '{}'", it->second, text);
	}

	std::string_view NameId::ToString() const
	{
		if (m_hash == 0)
			return {};

		InternTable& table = Table();
		ScopedLock lock(table.mutex);

		auto it = table.entries.find(m_hash);

		if (it == table.entries.end())
		{
			Terminal::Error(StringOps::GetName(this), "Hash {} has no interned string", m_hash);
			return {};
		}

		return it->second;
	}
}