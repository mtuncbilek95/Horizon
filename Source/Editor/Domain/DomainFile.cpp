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

	DomainFile::DomainSubAsset* DomainFile::FindSubAsset(std::string_view key)
	{
		for (auto& subAsset : m_subAssets)
		{
			if (subAsset.key == key)
				return &subAsset;
		}

		Terminal::Error(StringOps::GetName(this), "Could not find {} in {}", key, m_name);
		return nullptr;
	}

	std::string DomainFile::GetSubAssetCallPath(std::string_view key) const
	{
		if (key.empty())
			return "/" + GetRelativePath();

		return "/" + GetRelativePath() + "/" + std::string(key);
	}

	std::string DomainFile::GetSubAssetName(std::string_view key) const
	{
		if (!key.empty())
			return m_name + "/" + std::string(key);

		if (!m_parent || !m_parent->GetParent())
			return m_name;

		return m_parent->GetName() + "/" + m_name;
	}

	Guid DomainFile::GetSubAssetId(std::string_view key) const
	{
		if (key.empty())
			return m_id;

		for (const DomainSubAsset& subAsset : m_subAssets)
		{
			if (subAsset.key == key)
				return subAsset.id;
		}

		return Guid();
	}

	void DomainFile::RestoreSubAsset(const std::string& key, const Guid& id, Reflect::Type* pAssetType)
	{
		for (DomainSubAsset& subAsset : m_subAssets)
		{
			if (subAsset.key != key)
				continue;

			subAsset.id = id;
			subAsset.pAssetType = pAssetType;
			return;
		}

		DomainSubAsset subAsset;
		subAsset.key = key;
		subAsset.id = key.empty() ? m_id : id;
		subAsset.pAssetType = pAssetType;
		m_subAssets.PushBack(std::move(subAsset));
	}

	void DomainFile::SyncSubAssets(const List<ImportDeclaredProduct>& newProducts, Engine::ReflectionSystem* pReflection, List<Guid>& outGuids)
	{
		List<DomainSubAsset> next;
		next.Reserve(newProducts.GetCount());

		for (const ImportDeclaredProduct& decl : newProducts)
		{
			Reflect::Type* pType = pReflection->GetType(decl.assetType);
			if (!pType)
			{
				Terminal::Error(StringOps::GetName(this), "{} product {} has an unknown asset type, skipped", m_name, decl.key);
				continue;
			}

			DomainSubAsset subAsset;
			subAsset.key = decl.key;
			subAsset.pAssetType = pType;
			subAsset.id = GetSubAssetId(decl.key);

			if (!subAsset.id.IsValid())
				subAsset.id = Guid::Generate();

			next.PushBack(std::move(subAsset));
		}

		for (const DomainSubAsset& old : m_subAssets)
		{
			b8 stillThere = false;
			for (const DomainSubAsset& fresh : next)
			{
				if (fresh.key == old.key)
				{
					stillThere = true;
					break;
				}
			}

			if (!stillThere)
				outGuids.PushBack(old.id);
		}

		m_subAssets = std::move(next);
	}

	b8 DomainFile::GetDependsOn(std::string_view relativePath) const
	{
		for (const DomainDependency& dependency : m_dependencies)
		{
			if (StringOps::EqualsNoCase(dependency.relativePath, relativePath))
				return true;
		}

		return false;
	}

	b8 DomainFile::HasUnresolvedDependency(std::string_view relativePath) const
	{
		for (const DomainDependency& dependency : m_dependencies)
		{
			if (!dependency.resolved && dependency.relativePath == relativePath)
				return true;
		}

		return false;
	}
}