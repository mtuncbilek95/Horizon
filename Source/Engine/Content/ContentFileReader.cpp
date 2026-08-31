#include "ContentFileReader.h"

#include <Runtime/Log/Terminal.h>

#include <cstring>

namespace Horizon::Engine
{
	namespace
	{
		constexpr u64 GuidByteSize = 16;
	}

	ContentFileReader::ContentFileReader(const void* pData, usize size) : m_data((const u8*)pData), m_size(size)
	{
		m_valid = Parse();
	}

	b8 ContentFileReader::Parse()
	{
		if (m_data == nullptr || m_size < sizeof(ContentFileHeader))
		{
			Terminal::Error("ContentFileReader", "Content file is smaller than its header");
			return false;
		}

		std::memcpy(&m_header, m_data, sizeof(ContentFileHeader));

		if (m_header.magic != ContentFileMagic)
		{
			Terminal::Error("ContentFileReader", "Content file carries the magic {} instead of {}", m_header.magic,
				ContentFileMagic);
			return false;
		}

		if (m_header.containerVersion > ContentFileVersion)
		{
			Terminal::Error("ContentFileReader", "Content file is version {} but only {} is supported",
				m_header.containerVersion, ContentFileVersion);
			return false;
		}

		if (m_header.typeNameOffset + sizeof(u32) > m_size)
		{
			Terminal::Error("ContentFileReader", "Content file carries no asset type name");
			return false;
		}

		u32 nameLength = 0;
		std::memcpy(&nameLength, m_data + m_header.typeNameOffset, sizeof(nameLength));

		if (m_header.typeNameOffset + sizeof(u32) + nameLength > m_size)
		{
			Terminal::Error("ContentFileReader", "Content file carries a truncated asset type name");
			return false;
		}

		m_assetTypeName.assign((const c8*)(m_data + m_header.typeNameOffset + sizeof(u32)), nameLength);

		const u64 dependencyBytes = (u64)m_header.dependencyCount * GuidByteSize;

		if (m_header.dependencyOffset + dependencyBytes > m_size)
		{
			Terminal::Error("ContentFileReader", "{} carries a truncated dependency table", m_assetTypeName);
			return false;
		}

		m_dependencies.Reserve(m_header.dependencyCount);

		for (u32 i = 0; i < m_header.dependencyCount; i++)
		{
			const u8* pCursor = m_data + m_header.dependencyOffset + (u64)i * GuidByteSize;

			u32 a = 0;
			u16 b = 0;
			u16 c = 0;
			u8 d[8] = {};

			std::memcpy(&a, pCursor, sizeof(a));
			std::memcpy(&b, pCursor + 4, sizeof(b));
			std::memcpy(&c, pCursor + 6, sizeof(c));
			std::memcpy(d, pCursor + 8, sizeof(d));

			m_dependencies.PushBack(Guid(a, b, c, d));
		}

		const u64 tableBytes = (u64)m_header.sectionCount * sizeof(ContentSectionDesc);

		if (m_header.sectionTableOffset + tableBytes > m_size)
		{
			Terminal::Error("ContentFileReader", "{} carries a truncated section table", m_assetTypeName);
			return false;
		}

		m_sections.Resize(m_header.sectionCount);

		if (m_header.sectionCount > 0)
			std::memcpy(m_sections.GetData(), m_data + m_header.sectionTableOffset, (usize)tableBytes);

		for (const ContentSectionDesc& section : m_sections)
		{
			if (section.dataOffset + section.storedSize <= m_size)
				continue;

			Terminal::Error("ContentFileReader", "{} section {} reaches past the file", m_assetTypeName, section.sectionId);
			return false;
		}

		return true;
	}

	const ContentSectionDesc* ContentFileReader::FindSection(u32 sectionId) const
	{
		for (const ContentSectionDesc& section : m_sections)
		{
			if (section.sectionId == sectionId)
				return &section;
		}

		Terminal::Error("ContentFileReader", "{} carries no section {}", m_assetTypeName, sectionId);
		return nullptr;
	}

	b8 ContentFileReader::ReadSection(u32 sectionId, List<u8>& outBytes) const
	{
		if (!m_valid)
			return false;

		const ContentSectionDesc* pSection = FindSection(sectionId);

		if (pSection == nullptr)
			return false;

		outBytes.Clear();
		outBytes.Resize((usize)pSection->storedSize);

		if (pSection->storedSize > 0)
			std::memcpy(outBytes.GetData(), m_data + pSection->dataOffset, (usize)pSection->storedSize);

		return true;
	}
}