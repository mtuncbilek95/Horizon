#pragma once

#include <Engine/Content/ContentFile.h>
#include <Runtime/Containers/Guid.h>
#include <Runtime/Containers/List.h>

#include <string>
#include <string_view>

namespace Horizon::Engine
{
	class H_EXPORT ContentFileWriter
	{
	public:
		ContentFileWriter(const Guid& guid, std::string_view assetTypeName) : m_guid(guid), m_assetTypeName(assetTypeName)
		{
		}

		const Guid& GetID() const { return m_guid; }

		void SetPayloadVersion(u32 version) { m_payloadVersion = version; }
		void AddDependency(const Guid& guid);
		void AddSection(u32 sectionId, u32 flags, const void* pData, u64 size);

		b8 Build(List<u8>& outBytes) const;

	private:
		struct PendingSection
		{
			u32 sectionId = kInvalid32;
			u32 flags = 0;
			List<u8> data;
		};

		Guid m_guid;
		std::string m_assetTypeName;
		u32 m_payloadVersion = 0;

		List<Guid> m_dependencies;
		List<PendingSection> m_sections;
	};
}