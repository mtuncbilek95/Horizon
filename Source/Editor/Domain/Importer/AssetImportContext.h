#pragma once

#include <Runtime/Containers/Guid.h>

#include <filesystem>

namespace Horizon
{
	class AssetImportContext
	{
	public:
		AssetImportContext(const std::filesystem::path& source, const Guid& guid);

		const Guid& AssetGuid() const { return m_guid; }
		const std::filesystem::path& SourcePath() const { return m_source; }
		const std::filesystem::path& BinaryPath() const { return m_binaryPath; }

		void SetBinaryPath(const std::filesystem::path& path) { m_binaryPath = path; }


	private:
		Guid m_guid;
		std::filesystem::path m_source;
		std::filesystem::path m_binaryPath;
	};
}