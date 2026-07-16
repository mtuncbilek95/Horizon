#pragma once

#include <string>
#include <vector>
#include <cstring>

namespace Horizon
{
	struct AssetHeader
	{
		static constexpr u32 Magic = 0x485A4131; // 'HZA1'

		u32 magic = Magic;
		u32 version = 1;
		Guid guid;
		std::string type;

		u64 settingsOffset = 0, settingsSize = 0;
		u64 payloadOffset = 0, payloadSize = 0;

		std::vector<Guid> dependencies;

		u64 ComputeSize() const
		{
			return sizeof(u32) * 2 +
				sizeof(Guid) + sizeof(u32) + type.size() +
				sizeof(u64) * 4 + sizeof(u32) +
				dependencies.size() * sizeof(Guid);
		}
	};

	namespace Detail
	{
		template<typename T>
		void Append(std::vector<u8>& out, const T& value)
		{
			const u8* p = reinterpret_cast<const u8*>(&value);
			out.insert(out.end(), p, p + sizeof(T));
		}


		template<typename T>
		b8 ReadRaw(const u8*& cursor, const u8* end, T& out)
		{
			if (cursor + sizeof(T) > end)
				return false;

			std::memcpy(&out, cursor, sizeof(T));
			cursor += sizeof(T);
			return true;
		}
	}

	inline std::vector<u8> SerializeHeader(AssetHeader& header)
	{
		u64 headerSize = header.ComputeSize();
		header.settingsOffset = headerSize;
		header.payloadOffset = headerSize + header.settingsSize;

		std::vector<u8> out;
		out.reserve((usize)headerSize);

		Detail::Append(out, header.magic);
		Detail::Append(out, header.version);
		Detail::Append(out, header.guid);

		u32 typeLen = (u32)header.type.size();
		Detail::Append(out, typeLen);
		out.insert(out.end(), header.type.begin(), header.type.end());

		Detail::Append(out, header.settingsOffset);
		Detail::Append(out, header.settingsSize);
		Detail::Append(out, header.payloadOffset);
		Detail::Append(out, header.payloadSize);

		u32 depCount = (u32)header.dependencies.size();
		Detail::Append(out, depCount);

		for (const Guid& dep : header.dependencies)
			Detail::Append(out, dep);

		return out;
	}

	inline b8 DeserializeHeader(const u8* data, u64 size, AssetHeader& out)
	{
		const u8* cursor = data;
		const u8* end = data + size;

		if (!Detail::ReadRaw(cursor, end, out.magic) || out.magic != AssetHeader::Magic)
			return false;

		if (!Detail::ReadRaw(cursor, end, out.version))
			return false;
		if (!Detail::ReadRaw(cursor, end, out.guid))
			return false;

		u32 typeLen = 0;
		if (!Detail::ReadRaw(cursor, end, typeLen) || cursor + typeLen > end)
			return false;

		out.type.assign((const char*)(cursor), typeLen);
		cursor += typeLen;

		if (!Detail::ReadRaw(cursor, end, out.settingsOffset))
			return false;
		if (!Detail::ReadRaw(cursor, end, out.settingsSize))
			return false;
		if (!Detail::ReadRaw(cursor, end, out.payloadOffset))
			return false;
		if (!Detail::ReadRaw(cursor, end, out.payloadSize))
			return false;

		u32 depCount = 0;
		if (!Detail::ReadRaw(cursor, end, depCount))
			return false;

		out.dependencies.resize(depCount);
		for (u32 i = 0; i < depCount; ++i)
		{
			if (!Detail::ReadRaw(cursor, end, out.dependencies[i]))
				return false;
		}

		return true;
	}
}