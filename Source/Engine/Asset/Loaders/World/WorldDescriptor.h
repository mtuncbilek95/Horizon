#pragma once

#include <Engine/Asset/IAssetDescriptor.h>
#include <Runtime/Reflection/Reflect.h>

#include <string>

namespace Horizon
{
	class WorldDescriptor : public IAssetDescriptor
	{
		HORIZON_REFLECT;
	public:
		WorldDescriptor() = default;
		WorldDescriptor(const std::string& worldName, u32 entityCount);
		~WorldDescriptor() = default;

		std::string_view GetAssetType() const final { return "World"; }

		void Serialize(std::vector<u8>& out) const final;
		b8 Deserialize(const u8* data, u64 size) final;

		const std::string GetWorldName() const { return m_worldName; }
		u32 GetEntityCount() const { return m_entityCount; }

	private:
		std::string m_worldName;
		u32 m_entityCount = 0;
	};
}