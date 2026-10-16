#include "DomainFile.h"

#include <Editor/Domain/DomainFolder.h>
#include <Editor/ImportPipeline/AssetImportSettings.h>
#include <Engine/Asset/AssetHeader.h>
#include <Runtime/Containers/StringOps.h>
#include <Runtime/Log/Terminal.h>
#include <Runtime/PAL/File/File.h>

#include <cstring>
#include <utility>

namespace Horizon::Editor
{
	DomainFile::DomainFile(DomainFolder* pParent, const Guid& id, const std::string& name, const std::string& metaPath,
		const std::string& sourcePath, const std::string& cookFolder, Reflect::Type* pAssetType) : m_parent(pParent), m_id(id),
		m_metaPath(metaPath), m_sourcePath(sourcePath), m_cookFolder(cookFolder), m_assetType(pAssetType)
	{
		const usize dot = name.rfind('.');
		if (dot == std::string::npos)
		{
			m_name = name;
			return;
		}

		m_name = name.substr(0, dot);
		m_extension = name.substr(dot);
	}

	DomainFile::~DomainFile()
	{
		ReleaseSettings();
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

	b8 DomainFile::CreateSettings(Reflect::Type* pType)
	{
		if (pType->GetBaseId() != Reflect::TypeOf<AssetImportSettings>())
			return false;

		m_settings = (AssetImportSettings*)pType->Create();
		if (!m_settings)
			return false;

		return true;
	}

	void DomainFile::ReleaseSettings()
	{
		if (!m_settings)
			return;

		Memory::Allocator::Delete(m_settings);
		m_settings = nullptr;
	}

}