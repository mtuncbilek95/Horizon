#include "BinaryArchive.h"

#include <Runtime/Containers/StringOps.h>
#include <Runtime/Log/Terminal.h>

namespace Horizon
{
	constexpr Horizon::u32 HashArchiveName(std::string_view name)
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

	void BinaryArchiveWriter::AppendSize(usize value)
	{
		if (value > BinaryArchiveMaxField)
		{
			Terminal::Error(StringOps::GetName(this), "Count of {} does not fit a 32 bit field", value);
			m_hasError = true;
			value = 0;
		}

		AppendU32(u32(value));
	}

	void BinaryArchiveWriter::AlignPayload(usize alignment)
	{
		const usize remainder = m_buffer.GetCount() % alignment;

		if (remainder != 0)
			Grow(alignment - remainder);
	}

	b8 BinaryArchiveWriter::PatchSize(usize offset, usize value)
	{
		if (value > BinaryArchiveMaxField)
		{
			Terminal::Error(StringOps::GetName(this), "Payload of {} bytes does not fit a 32 bit size field", value);
			m_hasError = true;
			return false;
		}

		const u32 stored = u32(value);
		std::memcpy(m_buffer.GetData() + offset, &stored, sizeof(stored));

		return true;
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

		PatchSize(entryOffset, m_buffer.GetCount() - (entryOffset + sizeof(u32)));
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
			Terminal::Error(StringOps::GetName(this), "EndObject without a matching BeginObject");
			m_hasError = true;
			return;
		}

		const Frame frame = m_frames.Back();
		m_frames.PopBack();

		PatchSize(frame.sizeOffset, m_buffer.GetCount() - frame.payloadOffset);
		PatchSize(frame.countOffset, frame.entryCount);

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

		frame.sizeOffset = m_buffer.GetCount();
		AppendU32(0);

		AppendSize(count);
		frame.payloadOffset = m_buffer.GetCount();

		m_frames.PushBack(frame);
	}

	void BinaryArchiveWriter::EndArray()
	{
		if (m_frames.IsEmpty())
		{
			Terminal::Error(StringOps::GetName(this), "EndArray without a matching BeginArray");
			m_hasError = true;
			return;
		}

		const Frame frame = m_frames.Back();
		m_frames.PopBack();

		PatchSize(frame.sizeOffset, m_buffer.GetCount() - frame.payloadOffset);
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

		AppendSize(value.size());
		Append(value.data(), value.size());

		EndEntry(entryOffset);
	}

	void BinaryArchiveWriter::WriteBytes(const void* pData, usize size)
	{
		const usize entryOffset = BeginEntry();

		AppendSize(size);
		AlignPayload(BinaryArchiveBlobAlignment);
		Append(pData, size);

		EndEntry(entryOffset);
	}

	BinaryArchiveReader::BinaryArchiveReader(const void* pData, usize size) : m_data(static_cast<const u8*>(pData)),
		m_size(size)
	{
		if (!m_data || m_size < 2 * sizeof(u32))
		{
			Terminal::Error(StringOps::GetName(this), "Archive is smaller than its own header");
			m_hasError = true;
			return;
		}

		const u32 magic = PeekU32(0);
		const u32 version = PeekU32(sizeof(u32));

		if (magic != BinaryArchiveMagic)
		{
			Terminal::Error(StringOps::GetName(this), "Archive magic {} does not match {}", magic, BinaryArchiveMagic);
			m_hasError = true;
			return;
		}

		if (version > BinaryArchiveVersion)
		{
			Terminal::Error(StringOps::GetName(this), "Archive version {} is newer than the supported {}", version, BinaryArchiveVersion);
			m_hasError = true;
			return;
		}

		m_version = version;
		m_cursor = 2 * sizeof(u32);
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
			Terminal::Error(StringOps::GetName(this), "Read of {} bytes at {} runs past the archive", size, m_cursor);
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

	usize BinaryArchiveReader::SeekBlob(usize& outSize)
	{
		const u32 storedSize = TakeU32();
		const usize remainder = m_cursor % BinaryArchiveBlobAlignment;

		if (remainder != 0)
			m_cursor += BinaryArchiveBlobAlignment - remainder;

		if (!CanRead(m_cursor, storedSize))
		{
			Terminal::Error(StringOps::GetName(this), "Blob of {} bytes runs past the archive", storedSize);
			m_hasError = true;
			outSize = 0;
			return BinaryArchiveInvalidOffset;
		}

		outSize = storedSize;
		return m_cursor;
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
			Terminal::Error(StringOps::GetName(this), "Object payload of {} bytes runs past the archive", payloadSize);
			m_hasError = true;
			frame.endOffset = m_size;
		}

		m_frames.PushBack(frame);
	}

	void BinaryArchiveReader::EndObject()
	{
		if (m_frames.IsEmpty())
		{
			Terminal::Error(StringOps::GetName(this), "EndObject without a matching BeginObject");
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
			Terminal::Error(StringOps::GetName(this), "Key '{}' was requested outside of an object", name);
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

		const u32 payloadSize = TakeU32();
		const u32 count = TakeU32();

		frame.endOffset = m_cursor + payloadSize;

		if (frame.endOffset > m_size)
		{
			Terminal::Error(StringOps::GetName(this), "Array payload of {} bytes runs past the archive", payloadSize);
			m_hasError = true;
			frame.endOffset = m_size;
		}

		m_frames.PushBack(frame);
		return count;
	}

	void BinaryArchiveReader::EndArray()
	{
		if (m_frames.IsEmpty())
		{
			Terminal::Error(StringOps::GetName(this), "EndArray without a matching BeginArray");
			m_hasError = true;
			return;
		}

		m_cursor = m_frames.Back().endOffset;
		m_frames.PopBack();
	}

	b8 BinaryArchiveReader::Seek(usize offset)
	{
		if (!m_frames.IsEmpty())
		{
			Terminal::Error(StringOps::GetName(this), "Seek was requested while {} frames are still open", m_frames.GetCount());
			return false;
		}

		if (offset > m_size)
		{
			Terminal::Error(StringOps::GetName(this), "Seek to {} runs past the archive", offset);
			m_hasError = true;
			return false;
		}

		m_cursor = offset;
		return true;
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
			Terminal::Error(StringOps::GetName(this), "String of {} bytes runs past the archive", length);
			m_hasError = true;
			return {};
		}

		std::string value(reinterpret_cast<const c8*>(m_data + m_cursor), length);
		m_cursor += length;

		return value;
	}

	usize BinaryArchiveReader::ReadBytes(void* pData, usize size)
	{
		usize storedSize = 0;
		const usize offset = SeekBlob(storedSize);

		if (offset == BinaryArchiveInvalidOffset)
			return 0;

		if (storedSize > size)
		{
			Terminal::Error(StringOps::GetName(this), "Blob of {} bytes does not fit the {} byte destination", storedSize, size);
			m_hasError = true;
			m_cursor += storedSize;
			return 0;
		}

		std::memcpy(pData, m_data + offset, storedSize);
		m_cursor += storedSize;

		return storedSize;
	}

	usize BinaryArchiveReader::ViewBytes(usize& outSize)
	{
		const usize offset = SeekBlob(outSize);

		if (offset == BinaryArchiveInvalidOffset)
			return offset;

		m_cursor += outSize;
		return offset;
	}
}