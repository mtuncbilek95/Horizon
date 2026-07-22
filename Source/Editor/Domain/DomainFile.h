#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>
#include <Runtime/RTTR/Reflection.h>

#include <filesystem>
#include <string>
#include <vector>
#include <span>

namespace Horizon
{
	class Engine;
	class DomainFolder;

	struct DomainFileDesc
	{
		DomainFolder* parentFolder = nullptr;

		std::filesystem::path metaPath;
		std::string name;
	};

	class H_EXPORT DomainFile final
	{
		friend class DomainSystem;
	public:
		DomainFile(const DomainFileDesc& desc, Engine* pEngine);
		~DomainFile();

		Engine* GetEngine() const { return m_engine; }
		DomainFolder* GetParent() const { return m_parentFolder; }

		const std::string& GetName() const { return m_name; }
		const std::filesystem::path& GetMetaPath() const { return m_metaPath; }
		const std::filesystem::path& GetSourcePath() const { return m_sourcePath; }

	private:
		Engine* m_engine = nullptr;
		DomainFolder* m_parentFolder = nullptr;

		std::filesystem::path m_metaPath;
		std::filesystem::path m_sourcePath;
		std::string m_name;


	};
}