#include "MaterialAssetLoader.h"

#include <Engine/Asset/AssetHeader.h>
#include <Engine/Asset/AssetService.h>
#include <Engine/Asset/Shader/ShaderAsset.h>
#include <Engine/Asset/Texture/TextureAsset.h>

#include <Runtime/Containers/StringOps.h>
#include <Runtime/Log/Terminal.h>
#include <Runtime/PAL/File/File.h>

#include <cstring>

namespace Horizon::Engine
{
	void MaterialAssetLoader::LoadAsync(MaterialAsset* pAsset)
	{
	}

	void MaterialAssetLoader::UnloadAsync(MaterialAsset* pAsset)
	{
	}

	void MaterialAssetLoader::LoadSync(MaterialAsset* pAsset)
	{
		pAsset->m_residency.Store(AssetResidency::Reading, PAL::MemoryOrder::Relaxed);

		PAL::FileAccessRequest fileHandl = PAL::File::RequestAccess(pAsset->m_cookPath, PAL::FileOperationAccessPolicy::Read, PAL::FileOperationSharePolicy::SharedRead);
		if (!fileHandl.IsValid())
		{
			Terminal::Error(StringOps::GetName(this), "{} Path: {}", "Access request to file has failed!", pAsset->m_id.ToString());
			pAsset->m_residency.Store(AssetResidency::Failed, PAL::MemoryOrder::Release);
			return;
		}

		List<u8> headerBytes;
		if (!PAL::File::ReadMemory(fileHandl, headerBytes, 0, sizeof(AssetHeader)) || headerBytes.GetCount() < sizeof(AssetHeader))
		{
			Terminal::Error(StringOps::GetName(this), "{} Path: {}", "Failed to read the AssetHeader", pAsset->m_id.ToString());
			pAsset->m_residency.Store(AssetResidency::Failed, PAL::MemoryOrder::Release);
			PAL::File::ReleaseAccess(fileHandl);
			return;
		}

		AssetHeader header;
		std::memcpy(&header, headerBytes.GetData(), sizeof(AssetHeader));

		if (header.magic != AssetHeader::Magic)
		{
			Terminal::Error(StringOps::GetName(this), "{} Path: {}", "Magic number doesn't match up!", pAsset->m_id.ToString());
			pAsset->m_residency.Store(AssetResidency::Failed, PAL::MemoryOrder::Release);
			PAL::File::ReleaseAccess(fileHandl);
			return;
		}

		if (header.version != AssetHeader::Version)
		{
			Terminal::Error(StringOps::GetName(this), "{} Path: {}", "Versions are not matching!", pAsset->m_id.ToString());
			pAsset->m_residency.Store(AssetResidency::Failed, PAL::MemoryOrder::Release);
			PAL::File::ReleaseAccess(fileHandl);
			return;
		}

		if (header.propertySize != sizeof(MaterialAssetProperties))
		{
			Terminal::Error(StringOps::GetName(this), "{} Path: {}", "MaterialAssetProperties are not matching!", pAsset->m_id.ToString());
			pAsset->m_residency.Store(AssetResidency::Failed, PAL::MemoryOrder::Release);
			PAL::File::ReleaseAccess(fileHandl);
			return;
		}

		List<u8> propertyBytes;
		if (!PAL::File::ReadMemory(fileHandl, propertyBytes, header.propertyOffset, header.propertyOffset + header.propertySize))
		{
			Terminal::Error(StringOps::GetName(this), "{} Path: {}", "Failed to read memory file for MaterialAssetProperties", pAsset->m_id.ToString());
			pAsset->m_residency.Store(AssetResidency::Failed, PAL::MemoryOrder::Release);
			PAL::File::ReleaseAccess(fileHandl);
			return;
		}

		pAsset->m_residency.Store(AssetResidency::Decoding, PAL::MemoryOrder::Relaxed);

		MaterialAssetProperties props;
		std::memcpy(&props, propertyBytes.GetData(), sizeof(MaterialAssetProperties));

		if (props.valueCount * sizeof(MaterialParamValue) != header.tableSize)
		{
			Terminal::Error(StringOps::GetName(this), "{} Path: {}", "Table size does not match the value count", pAsset->m_id.ToString());
			pAsset->m_residency.Store(AssetResidency::Failed, PAL::MemoryOrder::Release);
			PAL::File::ReleaseAccess(fileHandl);
			return;
		}

		List<MaterialParamValue> values(props.valueCount);

		if (props.valueCount > 0)
		{
			List<u8> tableBytes;
			if (!PAL::File::ReadMemory(fileHandl, tableBytes, header.tableOffset, header.tableOffset + header.tableSize))
			{
				Terminal::Error(StringOps::GetName(this), "{} Path: {}", "Failed to read the value table", pAsset->m_id.ToString());
				pAsset->m_residency.Store(AssetResidency::Failed, PAL::MemoryOrder::Release);
				PAL::File::ReleaseAccess(fileHandl);
				return;
			}

			std::memcpy(values.GetData(), tableBytes.GetData(), header.tableSize);
		}

		PAL::File::ReleaseAccess(fileHandl);

		auto* pAssetService = GetEngine()->RequestService<AssetService>();
		if (!pAssetService->HasAsset(props.shaderId))
		{
			Terminal::Warn(StringOps::GetName(this), "{} Path: {}", "Shader of the material is not registered", pAsset->m_id.ToString());
		}
		else
		{
			ShaderAsset* pShader = pAssetService->FindAsset<ShaderAsset>(props.shaderId);

			if (pShader)
				pShader->LoadSync();
		}

		for (const MaterialParamValue& value : values)
		{
			if (value.type != MaterialParamType::Texture || !value.textureId.IsValid())
				continue;

			if (!pAssetService->HasAsset(value.textureId))
			{
				Terminal::Warn(StringOps::GetName(this), "{} Path: {}", "a texture of the material is not registered", pAsset->m_id.ToString());
				continue;
			}

			TextureAsset* pTexture = pAssetService->FindAsset<TextureAsset>(value.textureId);

			if (pTexture)
				pTexture->LoadSync();
		}

		pAsset->m_properties = props;
		pAsset->m_values = std::move(values);
		pAsset->m_residency.Store(AssetResidency::Resident, PAL::MemoryOrder::Release);
	}

	void MaterialAssetLoader::UnloadSync(MaterialAsset* pAsset)
	{
		ReleaseAsset(pAsset);
	}

	void MaterialAssetLoader::OnInitialize()
	{
		m_jobSystem = GetEngine()->GetJobSystem();
	}

	void MaterialAssetLoader::OnRegisteredAsset(const AssetEntry& entry)
	{
		auto* pMaterialAsset = static_cast<MaterialAsset*>(entry.pAssetObject);

		pMaterialAsset->m_id = entry.assetId;
		pMaterialAsset->m_cookPath = entry.cookPath;
	}

	void MaterialAssetLoader::OnUnregisteredAsset(const AssetEntry& entry)
	{
		ReleaseAsset(static_cast<MaterialAsset*>(entry.pAssetObject));
	}

	void MaterialAssetLoader::OnRefreshAsset(const AssetEntry& entry)
	{
		ReleaseAsset(static_cast<MaterialAsset*>(entry.pAssetObject));
	}

	void MaterialAssetLoader::ReleaseAsset(MaterialAsset* pAsset)
	{
		pAsset->m_values.Clear();
		pAsset->m_properties = {};
		pAsset->m_residency.Store(AssetResidency::Unloaded, PAL::MemoryOrder::Release);
	}
}