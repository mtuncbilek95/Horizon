#include "AssetTypeRegistry.h"

#include <cassert>

namespace Horizon
{
	u16 AssetTypeRegistry::Register(const AssetTypeDesc& desc)
	{
		assert(desc.id != AssetTypeId::Invalid);
		const u16 index = static_cast<u16>(m_types.size());

		auto [it, inserted] = m_byId.try_emplace(desc.id, index);
		assert(inserted && "AssetTypeId collision or double register");

		m_types.push_back(desc);
		return index;
	}

	u16 AssetTypeRegistry::IndexOf(AssetTypeId id) const
	{
		auto it = m_byId.find(id);
		return it != m_byId.end() ? it->second : kInvalid16;
	}

}