#pragma once

#include <Engine/Asset/AssetSourceFile.h>
#include <Engine/Asset/AssetEntry.h>
#include <Runtime/Containers/Guid.h>

#include <string>
#include <string_view>
#include <unordered_map>

namespace Horizon::Engine
{
	class H_EXPORT LooseSourceFile final : public AssetSourceFile
	{
	public:
		LooseSourceFile(const std::string& name);
		~LooseSourceFile() = default;

		std::string_view GetName() const final { return m_name; }

		const AssetEntry* Find(const Guid& id) const final;

		b8 Register(const AssetEntry& entry);
		b8 Unregister(const Guid& id);
		void Clear();

		usize GetCount() const { return m_entries.size(); }

	private:
		std::string m_name;
		std::unordered_map<Guid, AssetEntry> m_entries;
	};
}