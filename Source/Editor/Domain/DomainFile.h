#pragma once

#include <Runtime/Containers/Guid.h>
#include <Editor/Domain/DomainObject.h>

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

		std::string fileName;
	};

	class H_EXPORT DomainFile : public DomainObject
	{
	public:
		DomainFile(const DomainFileDesc& desc, Engine* pEngine);
		~DomainFile();

		const Guid& GetGuid() const { return m_id; }
		DomainFolder* GetParentFolder() const { return m_parent; }

		const std::string& GetName() const { return m_name; }
		const std::filesystem::path& GetSourcePath() const { return m_sourcePath; }
		const std::filesystem::path& GetMetaPath() const { return m_metaPath; }

	private:
		Guid m_id;

		Engine* m_engine;
		DomainFolder* m_parent;

		std::string m_name;
		std::filesystem::path m_sourcePath;
		std::filesystem::path m_metaPath;
	};
}