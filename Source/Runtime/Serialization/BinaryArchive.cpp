#include "BinaryArchive.h"

#include <Runtime/Log/Terminal.h>

namespace Horizon
{
	constexpr u32 HashArchiveName(std::string_view name)
	{
		u32 hash = 2166136261u;

		for (c8 character : name)
		{
			hash ^= u32(u8(character));
			hash *= 16777619u;
		}

		return hash;
	}

	BinaryArchiveWriter::BinaryArchiveWriter()
	{
		AppendU32(BinaryArchiveMagic);
		AppendU32(BinaryArchiveVersion);
	}

	usize BinaryArchiveWriter::Grow(usize size)
	{
		const usize offset = m_buffer.GetCount();
		const usize required = offset + size;

		if (required > m_buffer.GetCapacity())
		{
			usize capacity = m_buffer.GetCapacity() == 0 ? 256 : m_buffer.GetCapacity();

			while (capacity < required)
				capacity *= 2;

			m_buffer.Reserve(capacity);
		}

		m_buffer.Resize(required);
		return offset;
	}

	void BinaryArchiveWriter::Append(const void* pData, usize size)
	{
		if (size == 0)
			return;

		const usize offset = Grow(size);
		std::memcpy(m_buffer.GetData() + offset, pData, size);
	}

	void BinaryArchiveWriter::AppendU32(u32 value)
	{
		Append(&value, sizeof(value));
	}

	void BinaryArchiveWriter::AlignPayload(usize alignment)
	{
		const usize remainder = m_buffer.GetCount() % alignment;

		if (remainder != 0)
			Grow(alignment - remainder);
	}

	void BinaryArchiveWriter::Patch(usize offset, u32 value)
	{
		std::memcpy(m_buffer.GetData() + offset, &value, sizeof(value));
	}

	usize BinaryArchiveWriter::BeginEntry()
	{
		if (!m_hasPendingKey)
			return BinaryArchiveInvalidOffset;

		AppendU32(m_pendingKeyHash);

		const usize sizeOffset = m_buffer.GetCount();
		AppendU32(0);

		m_hasPendingKey = false;

		if (!m_frames.IsEmpty())
			m_frames.Back().entryCount++;

		return sizeOffset;
	}

	void BinaryArchiveWriter::EndEntry(usize entryOffset)
	{
		if (entryOffset == BinaryArchiveInvalidOffset)
			return;

		Patch(entryOffset, u32(m_buffer.GetCount() - (entryOffset + sizeof(u32))));
	}

	void BinaryArchiveWriter::BeginObject()
	{
		Frame frame;
		frame.entryOffset = BeginEntry();

		frame.sizeOffset = m_buffer.GetCount();
		AppendU32(0);

		frame.countOffset = m_buffer.GetCount();
		AppendU32(0);

		frame.payloadOffset = m_buffer.GetCount();

		m_frames.PushBack(frame);
	}

	void BinaryArchiveWriter::EndObject()
	{
		if (m_frames.IsEmpty())
		{
			Terminal::Error("BinaryArchiveWriter", "EndObject without a matching BeginObject");
			return;
		}

		const Frame frame = m_frames.Back();
		m_frames.PopBack();

		Patch(frame.sizeOffset, u32(m_buffer.GetCount() - frame.payloadOffset));
		Patch(frame.countOffset, frame.entryCount);

		EndEntry(frame.entryOffset);
	}

	void BinaryArchiveWriter::Key(std::string_view name)
	{
		m_pendingKeyHash = HashArchiveName(name);
		m_hasPendingKey = true;
	}

	void BinaryArchiveWriter::BeginArray(usize count)
	{
		Frame frame;
		frame.entryOffset = BeginEntry();
		frame.payloadOffset = m_buffer.GetCount();

		AppendU32(u32(count));

		m_frames.PushBack(frame);
	}

	void BinaryArchiveWriter::EndArray()
	{
		if (m_frames.IsEmpty())
		{
			Terminal::Error("BinaryArchiveWriter", "EndArray without a matching BeginArray");
			return;
		}

		const Frame frame = m_frames.Back();
		m_frames.PopBack();

		EndEntry(frame.entryOffset);
	}

	void BinaryArchiveWriter::WriteBool(b8 value)
	{
		const usize entryOffset = BeginEntry();
		const u8 stored = value ? u8(1) : u8(0);

		Append(&stored, sizeof(stored));
		EndEntry(entryOffset);
	}

	void BinaryArchiveWriter::WriteI64(i64 value)
	{
		const usize entryOffset = BeginEntry();

		Append(&value, sizeof(value));
		EndEntry(entryOffset);
	}

	void BinaryArchiveWriter::WriteU64(u64 value)
	{
		const usize entryOffset = BeginEntry();

		Append(&value, sizeof(value));
		EndEntry(entryOffset);
	}

	void BinaryArchiveWriter::WriteF64(f64 value)
	{
		const usize entryOffset = BeginEntry();

		Append(&value, sizeof(value));
		EndEntry(entryOffset);
	}

	void BinaryArchiveWriter::WriteString(std::string_view value)
	{
		const usize entryOffset = BeginEntry();

		AppendU32(u32(value.size()));
		Append(value.data(), value.size());

		EndEntry(entryOffset);
	}

	void BinaryArchiveWriter::WriteBytes(const void* pData, usize size)
	{
		const usize entryOffset = BeginEntry();

		AppendU32(u32(size));
		AlignPayload(BinaryArchiveBlobAlignment);
		Append(pData, size);

		EndEntry(entryOffset);
	}

	BinaryArchiveReader::BinaryArchiveReader(const void* pData, usize size) : m_data(static_cast<const u8*>(pData)),
		m_size(size)
	{
		if (!m_data || m_size < 2 * sizeof(u32))
		{
			Terminal::Error("BinaryArchiveReader", "Archive is smaller than its own header");
			m_hasError = true;
			return;
		}

		const u32 magic = PeekU32(0);
		const u32 version = PeekU32(sizeof(u32));

		if (magic != BinaryArchiveMagic)
		{
			Terminal::Error("BinaryArchiveReader", "Archive magic {} does not match {}", magic, BinaryArchiveMagic);
			m_hasError = true;
			return;
		}

		if (version != BinaryArchiveVersion)
		{
			Terminal::Error("BinaryArchiveReader", "Archive version {} is not {}", version, BinaryArchiveVersion);
			m_hasError = true;
			return;
		}

		m_cursor = 2 * sizeof(u32);
		m_valueEnd = m_size;
	}

	b8 BinaryArchiveReader::CanRead(usize offset, usize size) const
	{
		if (offset > m_size)
			return false;

		return size <= m_size - offset;
	}

	b8 BinaryArchiveReader::Take(void* pOut, usize size)
	{
		if (!CanRead(m_cursor, size))
		{
			Terminal::Error("BinaryArchiveReader", "Read of {} bytes at {} runs past the archive", size, m_cursor);
			m_hasError = true;
			return false;
		}

		std::memcpy(pOut, m_data + m_cursor, size);
		m_cursor += size;
		return true;
	}

	u32 BinaryArchiveReader::TakeU32()
	{
		u32 value = 0;
		Take(&value, sizeof(value));
		return value;
	}

	u32 BinaryArchiveReader::PeekU32(usize offset) const
	{
		u32 value = 0;

		if (!CanRead(offset, sizeof(value)))
			return 0;

		std::memcpy(&value, m_data + offset, sizeof(value));
		return value;
	}

	void BinaryArchiveReader::BeginObject()
	{
		Frame frame;
		frame.isObject = true;

		const u32 payloadSize = TakeU32();
		frame.fieldCount = TakeU32();
		frame.fieldsOffset = m_cursor;
		frame.endOffset = m_cursor + payloadSize;

		if (frame.endOffset > m_size)
		{
			Terminal::Error("BinaryArchiveReader", "Object payload of {} bytes runs past the archive", payloadSize);
			m_hasError = true;
			frame.endOffset = m_size;
		}

		m_frames.PushBack(frame);
	}

	void BinaryArchiveReader::EndObject()
	{
		if (m_frames.IsEmpty())
		{
			Terminal::Error("BinaryArchiveReader", "EndObject without a matching BeginObject");
			m_hasError = true;
			return;
		}

		m_cursor = m_frames.Back().endOffset;
		m_frames.PopBack();
	}

	b8 BinaryArchiveReader::Key(std::string_view name)
	{
		if (m_frames.IsEmpty() || !m_frames.Back().isObject)
		{
			Terminal::Error("BinaryArchiveReader", "Key '{}' was requested outside of an object", name);
			return false;
		}

		const Frame& frame = m_frames.Back();
		const u32 wanted = HashArchiveName(name);

		usize offset = frame.fieldsOffset;

		for (u32 index = 0; index < frame.fieldCount; ++index)
		{
			if (!CanRead(offset, 2 * sizeof(u32)))
				break;

			const u32 hash = PeekU32(offset);
			const u32 payloadSize = PeekU32(offset + sizeof(u32));
			const usize payloadOffset = offset + 2 * sizeof(u32);

			if (hash == wanted)
			{
				m_cursor = payloadOffset;
				m_valueEnd = payloadOffset + payloadSize;
				return true;
			}

			offset = payloadOffset + payloadSize;
		}

		return false;
	}

	usize BinaryArchiveReader::BeginArray()
	{
		Frame frame;
		frame.isObject = false;
		frame.endOffset = m_valueEnd;

		const u32 count = TakeU32();

		m_frames.PushBack(frame);
		return count;
	}

	void BinaryArchiveReader::EndArray()
	{
		if (m_frames.IsEmpty())
		{
			Terminal::Error("BinaryArchiveReader", "EndArray without a matching BeginArray");
			m_hasError = true;
			return;
		}

		m_cursor = m_frames.Back().endOffset;
		m_frames.PopBack();
	}

	b8 BinaryArchiveReader::ReadBool()
	{
		u8 value = 0;
		Take(&value, sizeof(value));
		return value != 0;
	}

	i64 BinaryArchiveReader::ReadI64()
	{
		i64 value = 0;
		Take(&value, sizeof(value));
		return value;
	}

	u64 BinaryArchiveReader::ReadU64()
	{
		u64 value = 0;
		Take(&value, sizeof(value));
		return value;
	}

	f64 BinaryArchiveReader::ReadF64()
	{
		f64 value = 0.0;
		Take(&value, sizeof(value));
		return value;
	}

	std::string BinaryArchiveReader::ReadString()
	{
		const u32 length = TakeU32();

		if (!CanRead(m_cursor, length))
		{
			Terminal::Error("BinaryArchiveReader", "String of {} bytes runs past the archive", length);
			m_hasError = true;
			return {};
		}

		std::string value(reinterpret_cast<const c8*>(m_data + m_cursor), length);
		m_cursor += length;

		return value;
	}

	usize BinaryArchiveReader::ReadBytes(void* pData, usize size)
	{
		const u32 storedSize = TakeU32();
		const usize remainder = m_cursor % BinaryArchiveBlobAlignment;

		if (remainder != 0)
			m_cursor += BinaryArchiveBlobAlignment - remainder;

		if (!CanRead(m_cursor, storedSize))
		{
			Terminal::Error("BinaryArchiveReader", "Blob of {} bytes runs past the archive", storedSize);
			m_hasError = true;
			return 0;
		}

		if (storedSize > size)
		{
			Terminal::Error("BinaryArchiveReader", "Blob of {} bytes does not fit the {} byte destination", storedSize, size);
			m_hasError = true;
			m_cursor += storedSize;
			return 0;
		}

		std::memcpy(pData, m_data + m_cursor, storedSize);
		m_cursor += storedSize;

		return storedSize;
	}
}