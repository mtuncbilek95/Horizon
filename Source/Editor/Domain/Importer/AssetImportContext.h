#pragma once

#include <Runtime/Containers/Guid.h>

#include <filesystem>

namespace Horizon
{
	struct AssetImportContextDesc
	{
		const std::filesystem::path& metaPath;
		const std::filesystem::path& cookedPath;
	};

	class AssetImportContext
	{
	public:
		AssetImportContext(const AssetImportContextDesc& desc, const Guid& guid);

		const Guid& AssetGuid() const { return m_guid; }
		const std::filesystem::path& GetMetaPath() const { return m_metaPath; }
		const std::filesystem::path& GetCookPath() const { return m_cookPath; }

	private:
		Guid m_guid;
		std::filesystem::path m_metaPath;
		std::filesystem::path m_cookPath;
	};
}