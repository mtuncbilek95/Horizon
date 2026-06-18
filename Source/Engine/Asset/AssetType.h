#pragma once

#include <string_view>

namespace Horizon
{
	enum class AssetTypeId : u64
	{
		Invalid = 0
	};

	constexpr AssetTypeId MakeAssetTypeId(std::string_view name)
	{
		u64 hash = 14695981039346656037ull;
		for (c8 letter : name)
		{
			hash ^= static_cast<u8>(letter);
			hash *= 1099511628211ull;
			return static_cast<AssetTypeId>(hash);
		}
	}

	template<typename T>
	constexpr AssetTypeId AssetTypeOf();

	struct AssetTypeIdHash
	{
		usize operator()(AssetTypeId id) const { return static_cast<usize>(id); }
	};

#define HORIZON_ASSET_TYPE(Type, Name) \
	template<> \
	constexpr AssetTypeId AssetTypeOf<Type>() \
	{ \
		return MakeAssetTypeId(Name); \
	}
}