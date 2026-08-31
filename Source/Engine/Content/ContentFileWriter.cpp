#include "ContentFileWriter.h"

#include <Runtime/Log/Terminal.h>

#include <cstring>
#include <utility>

namespace Horizon::Engine
{
	namespace
	{
		constexpr u64 GuidByteSize = 16;

		u64 AlignUp(u64 value, u64 alignment)
		{
			return (value + alignment - 1) & ~(alignment - 1);
		}

		void Append(List<u8>& outBytes, const void* pData, u64 size)
		{
			if (size == 0)
				return;

			const usize offset = outBytes.GetCount();

			outBytes.Resize(offset + (usize)size);
			std::memcpy(outBytes.GetData() + offset, pData, (usize)size);
		}

		void AppendGuid(List<u8>& outBytes, const Guid& guid)
		{
			const u32 a = guid.GetA();
			const u16 b = guid.GetB();
			const u16 c = guid.GetC();

			Append(outBytes, &a, sizeof(a));
			Append(outBytes, &b, sizeof(b));
			Append(outBytes, &c, sizeof(c));
			Append(outBytes, guid.GetD(), 8);
		}

		void PadTo(List<u8>& outBytes, u64 offset)
		{
			if ((u64)outBytes.GetCount() >= offset)
				return;

			outBytes.Resize((usize)offset);
		}
	}

	void ContentFileWriter::AddDependency(const Guid& guid)
	{
		if (m_dependencies.Contains(guid))
			return;

		m_dependencies.PushBack(guid);
	}

	void ContentFileWriter::AddSection(u32 sectionId, u32 flags, const void* pData, u64 size)
	{
		PendingSection section;
		section.sectionId = sectionId;
		section.flags = flags;
		section.data.Resize((usize)size);

		if (size > 0)
			std::memcpy(section.data.GetData(), pData, (usize)size);

		m_sections.PushBack(std::move(section));
	}

	b8 ContentFileWriter::Build(List<u8>& outBytes) const
	{
		if (!m_guid.IsValid())
		{
			Terminal::Error("ContentFileWriter", "Content file carries no valid id");
			return false;
		}

		if (m_assetTypeName.empty())
		{
			Terminal::Error("ContentFileWriter", "{} carries no asset type name", m_guid.ToString());
			return false;
		}

		ContentFileHeader header;
		header.payloadVersion = m_payloadVersion;
		header.sectionCount = (u32)m_sections.GetCount();
		header.dependencyCount = (u32)m_dependencies.GetCount();
		header.id = m_guid;

		u64 cursor = sizeof(ContentFileHeader);

		header.typeNameOffset = cursor;
		cursor += sizeof(u32) + m_assetTypeName.size();

		header.dependencyOffset = cursor;
		cursor += (u64)m_dependencies.GetCount() * GuidByteSize;

		header.sectionTableOffset = AlignUp(cursor, 8);
		cursor = header.sectionTableOffset + (u64)m_sections.GetCount() * sizeof(ContentSectionDesc);

		List<ContentSectionDesc> table(m_sections.GetCount());

		for (usize i = 0; i < m_sections.GetCount(); ++i)
		{
			cursor = AlignUp(cursor, ContentSectionAlignment);

			table[i].sectionId = m_sections[i].sectionId;
			table[i].flags = m_sections[i].flags;
			table[i].dataOffset = cursor;
			table[i].storedSize = m_sections[i].data.GetCount();
			table[i].decodedSize = m_sections[i].data.GetCount();

			cursor += m_sections[i].data.GetCount();
		}

		outBytes.Clear();
		outBytes.Reserve((usize)cursor);

		Append(outBytes, &header, sizeof(header));

		const u32 nameLength = (u32)m_assetTypeName.size();

		Append(outBytes, &nameLength, sizeof(nameLength));
		Append(outBytes, m_assetTypeName.data(), nameLength);

		for (const Guid& dependency : m_dependencies)
			AppendGuid(outBytes, dependency);

		PadTo(outBytes, header.sectionTableOffset);
		Append(outBytes, table.GetData(), (u64)table.GetCount() * sizeof(ContentSectionDesc));

		for (usize i = 0; i < m_sections.GetCount(); ++i)
		{
			PadTo(outBytes, table[i].dataOffset);
			Append(outBytes, m_sections[i].data.GetData(), m_sections[i].data.GetCount());
		}

		return true;
	}
}