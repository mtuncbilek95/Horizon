#include "DomainFile.h"

#include <Editor/Domain/DomainFolder.h>
#include <Runtime/PAL/File/File.h>

namespace Horizon::Editor
{
	std::string DomainFile::GetRelativePath() const
	{
		const std::string parentPath = m_parent->GetRelativePath();

		if (parentPath.empty())
			return m_name;

		return parentPath + "/" + m_name;
	}

	b8 DomainFile::EnsureMeta()
	{
		if (m_meta.id.IsValid())
			return true;

		if (PAL::File::Exists(m_metaPath))
			return m_meta.Read(m_metaPath);

		m_meta.id = Guid::Generate();
		m_meta.assetTypeName.clear();
		m_meta.subAssets.Clear();

		return m_meta.Write(m_metaPath);
	}

	b8 DomainFile::ReloadMeta()
	{
		m_meta = DomainMeta();

		return EnsureMeta();
	}

	b8 DomainFile::SetAssetType(const std::string& assetTypeName)
	{
		if (m_meta.assetTypeName == assetTypeName)
			return true;

		m_meta.assetTypeName = assetTypeName;

		return m_meta.Write(m_metaPath);
	}

	void DomainFile::Rename(const std::string& newName)
	{
		std::string newPath = m_parent->GetAbsolutePath() + "/" + newName;
		PAL::File::Rename(m_sourcePath, newPath);
	}
}