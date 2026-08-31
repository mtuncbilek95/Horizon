#pragma once

#include <Engine/Content/ContentFile.h>
#include <Runtime/Containers/Guid.h>
#include <Runtime/Containers/List.h>

#include <string>

namespace Horizon::Engine
{
	class H_EXPORT ContentFileReader
	{
	public:
		ContentFileReader(const void* pData, usize size);

		b8 IsValid() const { return m_valid; }

		const Guid& GetID() const { return m_header.id; }
		u32 GetPayloadVersion() const { return m_header.payloadVersion; }
		const std::string& GetAssetTypeName() const { return m_assetTypeName; }

		const List<Guid>& GetDependencies() const { return m_dependencies; }
		const List<ContentSectionDesc>& GetSections() const { return m_sections; }

		const ContentSectionDesc* FindSection(u32 sectionId) const;
		b8 ReadSection(u32 sectionId, List<u8>& outBytes) const;

	private:
		b8 Parse();

		const u8* m_data = nullptr;
		usize m_size = 0;

		ContentFileHeader m_header;
		std::string m_assetTypeName;
		List<Guid> m_dependencies;
		List<ContentSectionDesc> m_sections;
		b8 m_valid = false;
	};
}