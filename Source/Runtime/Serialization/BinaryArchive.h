#pragma once

#include <Runtime/Containers/List.h>
#include <Runtime/Serialization/Archive.h>

#include <string>
#include <string_view>

namespace Horizon
{
	inline constexpr u32 BinaryArchiveMagic = 0x4E5A5248u;
	inline constexpr u32 BinaryArchiveVersion = 1u;
	inline constexpr usize BinaryArchiveBlobAlignment = 16;
	inline constexpr usize BinaryArchiveInvalidOffset = usize(kInvalid64);
	constexpr u32 HashArchiveName(std::string_view name);

	class H_EXPORT BinaryArchiveWriter final : public IArchiveWriter
	{
		struct Frame
		{
			usize entryOffset = BinaryArchiveInvalidOffset;
			usize sizeOffset = BinaryArchiveInvalidOffset;
			usize countOffset = BinaryArchiveInvalidOffset;
			usize payloadOffset = 0;
			u32 entryCount = 0;
		};
	public:
		BinaryArchiveWriter();

		void BeginObject() final;
		void EndObject() final;
		void Key(std::string_view name) final;
		void BeginArray(usize count) final;
		void EndArray() final;

		void WriteBool(b8 value) final;
		void WriteI64(i64 value) final;
		void WriteU64(u64 value) final;
		void WriteF64(f64 value) final;
		void WriteString(std::string_view value) final;
		void WriteBytes(const void* pData, usize size) final;

		const List<u8>& GetBytes() const { return m_buffer; }
		b8 IsBalanced() const { return m_frames.IsEmpty(); }

	private:
		usize BeginEntry();
		void EndEntry(usize entryOffset);

		usize Grow(usize size);
		void Append(const void* pData, usize size);
		void AppendU32(u32 value);
		void AlignPayload(usize alignment);
		void Patch(usize offset, u32 value);

	private:
		List<u8> m_buffer;
		List<Frame> m_frames;
		u32 m_pendingKeyHash = 0;
		b8 m_hasPendingKey = false;
	};

	class H_EXPORT BinaryArchiveReader final : public IArchiveReader
	{
		struct Frame
		{
			usize fieldsOffset = 0;
			usize endOffset = 0;
			u32 fieldCount = 0;
			b8 isObject = false;
		};
	public:
		BinaryArchiveReader(const void* pData, usize size);

		void BeginObject() final;
		void EndObject() final;
		b8 Key(std::string_view name) final;
		usize BeginArray() final;
		void EndArray() final;

		b8 ReadBool() final;
		i64 ReadI64() final;
		u64 ReadU64() final;
		f64 ReadF64() final;
		std::string ReadString() final;
		usize ReadBytes(void* pData, usize size) final;

		b8 HasError() const final { return m_hasError; }

	private:
		b8 CanRead(usize offset, usize size) const;
		b8 Take(void* pOut, usize size);
		u32 TakeU32();
		u32 PeekU32(usize offset) const;

	private:
		const u8* m_data = nullptr;
		usize m_size = 0;
		usize m_cursor = 0;
		usize m_valueEnd = 0;
		List<Frame> m_frames;
		b8 m_hasError = false;
	};
}