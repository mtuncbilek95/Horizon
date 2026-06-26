#pragma once

#include <Runtime/Containers/Guid.h>

#include <filesystem>
#include <string>

namespace Horizon
{
	class Engine;
	class DomainFolder;

	struct DomainFileDesc
	{
		Guid fileId;

		DomainFolder* pParent = nullptr;

		std::filesystem::path metaPath;
		std::filesystem::path binaryPath;
	};

	class H_EXPORT DomainFile
	{
	public:
		DomainFile(const DomainFileDesc& desc, Engine* pEngine);
		~DomainFile();

		const Guid& GetGuid() const { return m_id; }
		DomainFolder* GetParentFolder() const { return m_parent; }

		const std::filesystem::path& GetMetaPath() const { return m_metaPath; }
		const std::filesystem::path& GetBinaryPath() const { return m_binaryPath; }

		const std::string& GetName() const { return m_name; }

	private:
		Guid m_id;

		DomainFolder* m_parent;

		std::filesystem::path m_metaPath;
		std::filesystem::path m_binaryPath;

		std::string m_name;

		usize m_binarySize;
	};
}