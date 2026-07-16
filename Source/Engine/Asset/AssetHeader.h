#pragma once

#include <string_view>
#include <cstring>
#include <type_traits>

namespace Horizon
{
	struct AssetHeader
	{
		static constexpr u32 Magic = 0x485A4131;      // 'HZA1'
		static constexpr u32 CurrentVersion = 1;
		static constexpr usize MaxTypeLength = 64;

		u32 magic = Magic;
		u32 version = CurrentVersion;

		Guid guid;

		u64 depsOffset = 0;
		u64 descriptorOffset = 0;
		u64 descriptorSize = 0;
		u64 payloadOffset = 0;
		u64 payloadSize = 0;

		u32 depsCount = 0;
		u32 reserved = 0;

		c8 type[MaxTypeLength] = {};

		b8 IsValid() const { return magic == Magic; }

		std::string_view GetType() const
		{
			usize len = 0;
			while (len < MaxTypeLength && type[len] != '\0')
				++len;

			return std::string_view(type, len);
		}

		template<usize N>
		void SetType(const char(&value)[N])
		{
			static_assert(N - 1 <= MaxTypeLength, "Asset type name exceeds AssetHeader::MaxTypeLength");

			std::memset(type, 0, MaxTypeLength);
			std::memcpy(type, value, N - 1);
		}

		[[nodiscard]] b8 SetType(std::string_view value)
		{
			if (value.size() > MaxTypeLength)
			{
				Terminal::Error("AssetHeader", "Asset type '{}' is {} chars — max {}",
					value, value.size(), MaxTypeLength);
				return false;
			}

			std::memset(type, 0, MaxTypeLength);
			std::memcpy(type, value.data(), value.size());
			return true;
		}

	};

	static_assert(sizeof(AssetHeader) == 136, "AssetHeader size drifted — it is a wire format");
	static_assert(std::is_trivially_copyable_v<AssetHeader>, "AssetHeader is memcpy'd to/from disk");
}