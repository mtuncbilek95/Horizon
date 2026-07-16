#pragma once

#include <string_view>
#include <vector>

namespace Horizon
{
	class IAssetDescriptor
	{
	public:
		virtual ~IAssetDescriptor() = default;

		virtual std::string_view GetAssetType() const = 0;

		virtual void Serialize(std::vector<u8>& out) const = 0;
		virtual b8 Deserialize(const u8* data, u64 size) = 0;
	};
}