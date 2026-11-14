#include "InputActionMapLoader.h"

#include <Engine/Asset/AssetHeader.h>
#include <Engine/Asset/Input/InputActionMapSerializer.h>

#include <Runtime/Containers/StringHelper.h>
#include <Runtime/Log/Terminal.h>
#include <Runtime/PAL/File/File.h>
#include <Runtime/Serialization/BinaryArchive.h>

namespace Horizon::Asset
{
	void InputActionMapLoader::LoadSync(InputActionMap* pAsset)
	{
		pAsset->m_residency.Store(Engine::AssetResidency::Reading, PAL::MemoryOrder::Relaxed);

		List<u8> headerBytes;
		List<u8> dataPayload;

		PAL::FileAccessRequest fileHandl = PAL::File::RequestAccess(pAsset->m_cookPath, PAL::FileOperationAccessPolicy::Read, PAL::FileOperationSharePolicy::SharedRead);
		if (!fileHandl.IsValid())
		{
			Terminal::Error(StringHelper::GetTypeName(this), "{} Path: {}", "Access request to file has failed!", pAsset->m_id.ToString());
			pAsset->m_residency.Store(Engine::AssetResidency::Failed, PAL::MemoryOrder::Release);
			return;
		}

		if (!PAL::File::ReadMemory(fileHandl, headerBytes, 0, sizeof(Engine::AssetHeader)) || headerBytes.GetCount() < sizeof(Engine::AssetHeader))
		{
			Terminal::Error(StringHelper::GetTypeName(this), "{} Path: {}", "Failed to read the AssetHeader", pAsset->m_id.ToString());
			pAsset->m_residency.Store(Engine::AssetResidency::Failed, PAL::MemoryOrder::Release);
			PAL::File::ReleaseAccess(fileHandl);
			return;
		}

		Engine::AssetHeader header;
		std::memcpy(&header, headerBytes.GetData(), sizeof(Engine::AssetHeader));

		if (header.magic != Engine::AssetHeader::Magic)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "{} Path: {}", "Magic number doesn't match up!", pAsset->m_id.ToString());
			pAsset->m_residency.Store(Engine::AssetResidency::Failed, PAL::MemoryOrder::Release);
			PAL::File::ReleaseAccess(fileHandl);
			return;
		}

		if (header.version != Engine::AssetHeader::Version)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "{} Path: {}", "Versions are not matching!", pAsset->m_id.ToString());
			pAsset->m_residency.Store(Engine::AssetResidency::Failed, PAL::MemoryOrder::Release);
			PAL::File::ReleaseAccess(fileHandl);
			return;
		}

		if (header.payloadSize == 0)
		{
			Terminal::Info(StringHelper::GetTypeName(this), "Payload is empty. Moving on to reduce the errors");
			PAL::File::ReleaseAccess(fileHandl);
			pAsset->m_actions.Clear();
			pAsset->m_residency.Store(Engine::AssetResidency::Resident, PAL::MemoryOrder::Release);
			return;
		}

		if (!PAL::File::ReadMemory(fileHandl, dataPayload, header.payloadOffset, header.payloadOffset + header.payloadSize))
		{
			Terminal::Error(StringHelper::GetTypeName(this), "{} Path: {}", "Failed to read memory file for BinaryData", pAsset->m_id.ToString());
			pAsset->m_residency.Store(Engine::AssetResidency::Failed, PAL::MemoryOrder::Release);
			PAL::File::ReleaseAccess(fileHandl);
			return;
		}

		PAL::File::ReleaseAccess(fileHandl);

		pAsset->m_residency.Store(Engine::AssetResidency::Decoding, PAL::MemoryOrder::Relaxed);

		BinaryArchiveReader reader(dataPayload.GetData(), dataPayload.GetCount());
		if (!InputActionMapSerializer::Deserialize(pAsset->m_actions, reader))
		{
			Terminal::Error(StringHelper::GetTypeName(this), "{} Path: {}", "Failed to deserialize the InputActionMap", pAsset->m_id.ToString());
			pAsset->m_actions.Clear();
			pAsset->m_residency.Store(Engine::AssetResidency::Failed, PAL::MemoryOrder::Release);
			return;
		}

		pAsset->m_residency.Store(Engine::AssetResidency::Resident, PAL::MemoryOrder::Release);
	}

	void InputActionMapLoader::UnloadSync(InputActionMap* pAsset)
	{
		pAsset->m_actions.Clear();
		pAsset->m_residency.Store(Engine::AssetResidency::Unloaded, PAL::MemoryOrder::Release);
	}

	void InputActionMapLoader::OnInitialize()
	{
	}

	void InputActionMapLoader::OnFinalize()
	{
	}

	void InputActionMapLoader::OnRegisteredAsset(const Engine::AssetEntry& entry)
	{
		auto* pMapAsset = static_cast<InputActionMap*>(entry.pAssetObject);

		pMapAsset->m_id = entry.assetId;
		pMapAsset->m_cookPath = entry.cookPath;
	}

	void InputActionMapLoader::OnUnregisteredAsset(const Engine::AssetEntry& entry)
	{
	}

	void InputActionMapLoader::OnRefreshAsset(const Engine::AssetEntry& entry)
	{
		auto* pMapAsset = static_cast<InputActionMap*>(entry.pAssetObject);

		pMapAsset->m_residency.CompareExchange(Engine::AssetResidency::Failed, Engine::AssetResidency::Unloaded);
	}
}
