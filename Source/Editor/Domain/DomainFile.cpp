#include "DomainFile.h"

#include <Editor/Domain/DomainFolder.h>
#include <Engine/Asset/AssetHeader.h>
#include <Runtime/Containers/StringOps.h>
#include <Runtime/Log/Terminal.h>
#include <Runtime/PAL/File/File.h>

#include <cstring>
#include <utility>

namespace Horizon::Editor
{
	DomainFile::DomainFile(DomainFolder* pParent, const std::string& name, const std::string& metaPath, 
		const std::string& sourcePath, const std::string& cookFolder, const std::string& assetType) : m_parent(pParent),
		m_name(name.substr(0, m_name.find_last_of('.'))), m_extension(name.substr(m_name.find_last_of('.'))), 
		m_metaPath(metaPath), m_sourcePath(sourcePath), m_cookFolder(cookFolder), m_assetTypeName(assetType)
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

	void DomainFile::Rename(const std::string& newName)
	{
		const std::string newPath = m_parent->GetAbsolutePath() + "/" + newName + m_extension;

		if (!PAL::File::Rename(m_sourcePath, newPath))
			Terminal::Error(StringOps::GetName(this), "{} could not be renamed to {}", m_sourcePath, newPath);
	}
}