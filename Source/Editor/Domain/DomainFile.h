#pragma once

#include <Runtime/Containers/Guid.h>
#include <string>

namespace Horizon::Editor
{
	class DomainFolder;

	class H_EXPORT DomainFile
	{
		friend class DomainSystem;
	public:
		DomainFile(const Guid& guid, DomainFolder* pParent, const std::string& name, const std::string& metaPath, const std::string& sourcePath /* Maybe later: cookPath*/) :
			m_guid(guid), m_parent(pParent), m_name(name), m_metaPath(metaPath), m_sourcePath(sourcePath)
		{
		}
		~DomainFile()
		{
		}

		const Guid& GetID() const { return m_guid; }
		DomainFolder* GetParent() const { return m_parent; }

		const std::string& GetName() const { return m_name; }

		const std::string& GetMetaPath() const { return m_metaPath; }
		const std::string& GetSourcePath() const { return m_sourcePath; }

		void Rename(const std::string& newName);

	private:
		Guid m_guid;
		DomainFolder* m_parent = nullptr;

		std::string m_name;

		std::string m_metaPath;
		std::string m_sourcePath;
	};
}