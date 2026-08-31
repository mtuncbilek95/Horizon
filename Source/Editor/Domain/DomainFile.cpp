#include "DomainFile.h"

#include <Editor/Domain/DomainFolder.h>
#include <Runtime/PAL/File/File.h>

#include <utility>

namespace Horizon::Editor
{
	DomainFile::DomainFile(DomainFolder* pParent, const std::string& name, const std::string& metaPath, const std::string& sourcePath) :
		m_parent(pParent), m_name(name), m_metaPath(metaPath), m_sourcePath(sourcePath)
	{
	}

	DomainFile::~DomainFile()
	{
	}

	std::string DomainFile::GetRelativePath() const
	{
		const std::string parentPath = m_parent->GetRelativePath();

		if (parentPath.empty())
			return m_name;

		return parentPath + "/" + m_name;
	}

	b8 DomainFile::HasMeta() const
	{
		return PAL::File::Exists(m_metaPath);
	}

	b8 DomainFile::LoadMeta()
	{
		if (!PAL::File::Exists(m_metaPath))
			return false;

		DomainMeta meta;

		if (!meta.Read(m_metaPath))
			return false;

		m_meta = std::move(meta);

		return true;
	}

	b8 DomainFile::WriteMeta(const DomainMeta& meta)
	{
		if (!meta.Write(m_metaPath))
			return false;

		m_meta = meta;

		return true;
	}

	void DomainFile::Rename(const std::string& newName)
	{
		std::string newPath = m_parent->GetAbsolutePath() + "/" + newName;
		PAL::File::Rename(m_sourcePath, newPath);
	}
}