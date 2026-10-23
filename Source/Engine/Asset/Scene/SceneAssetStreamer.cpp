#include "SceneAssetStreamer.h"

#include <Engine/Asset/AssetHeader.h>
#include <Engine/Asset/Scene/SceneSerializer.h>
#include <Engine/Asset/Scene/SceneProperties.h>

#include <Runtime/Containers/ScopedLock.h>
#include <Runtime/Containers/StringOps.h>
#include <Runtime/Log/Terminal.h>
#include <Runtime/PAL/File/File.h>
#include <Runtime/Serialization/BinaryArchive.h>

#include <cstring>

namespace Horizon::Engine
{
	void SceneAssetStreamer::OnInitialize()
	{
		m_jobSystem = GetEngine()->GetJobSystem();
	}

	void SceneAssetStreamer::OnRegisteredAsset(const AssetEntry& entry)
	{
		auto* pSceneAsset = static_cast<SceneAsset*>(entry.pAssetObject);

		pSceneAsset->m_id = entry.assetId;
		pSceneAsset->m_cookPath = entry.cookPath;
	}

	void SceneAssetStreamer::OnUnregisteredAsset(const AssetEntry& entry)
	{
		auto* pSceneAsset = static_cast<SceneAsset*>(entry.pAssetObject);

		if (pSceneAsset->m_ticket != InvalidSubmitTicket)
			m_jobSystem->WaitTicket(pSceneAsset->m_ticket);
	}

	void SceneAssetStreamer::OnRefreshAsset(const AssetEntry& entry)
	{
		auto* pSceneAsset = static_cast<SceneAsset*>(entry.pAssetObject);

		pSceneAsset->m_residency.CompareExchange(AssetResidency::Failed, AssetResidency::Unloaded);
	}

	void SceneAssetStreamer::LoadAsync(SceneAsset* pAsset)
	{
		if (!pAsset->m_scene)
			pAsset->m_scene = Memory::Allocator::Create<Scene>(Memory::CurrLoc(), GetEngine()->GetReflectionSystem());

		pAsset->m_ticket = m_jobSystem->SubmitJob(JobLane::Background, Job([this, pAsset]()
			{
				RunLoadWorld(pAsset);
			}));

		// Nearly impossible but sure
		if (pAsset->m_ticket == InvalidSubmitTicket)
		{
			Terminal::Error(StringOps::GetName(this), "{} Path: {}", "Job could not be submitted", pAsset->m_id.ToString());
			pAsset->m_residency.Store(AssetResidency::Failed, PAL::MemoryOrder::Release);
		}
	}

	void SceneAssetStreamer::UnloadAsync(SceneAsset* pAsset)
	{
	}

	void SceneAssetStreamer::RunLoadWorld(SceneAsset* pAsset)
	{
		pAsset->m_residency.Store(AssetResidency::Reading, PAL::MemoryOrder::Relaxed);

		List<u8> headerBytes;
		List<u8> propertyBytes;
		List<u8> dataPayload;

		// Read payload then put up your ass.
		PAL::FileAccessRequest fileHandl = PAL::File::RequestAccess(pAsset->m_cookPath, PAL::FileOperationAccessPolicy::Read, PAL::FileOperationSharePolicy::SharedRead);
		if (!fileHandl.IsValid())
		{
			Terminal::Error(StringOps::GetName(this), "{} Path: {}", "Access request to file has failed!", pAsset->m_id.ToString());
			pAsset->m_residency.Store(AssetResidency::Failed, PAL::MemoryOrder::Release);
			return;
		}

		if (!PAL::File::ReadMemory(fileHandl, headerBytes, 0, sizeof(AssetHeader)) || headerBytes.GetCount() < sizeof(AssetHeader))
		{
			Terminal::Error(StringOps::GetName(this), "{} Path: {}", "Failed to read the AssetHeader", pAsset->m_id.ToString());
			pAsset->m_residency.Store(AssetResidency::Failed, PAL::MemoryOrder::Release);
			PAL::File::ReleaseAccess(fileHandl);
			return;
		}

		AssetHeader header;
		std::memcpy(&header, headerBytes.GetData(), sizeof(AssetHeader));

		// Fail checks (Doesn't happen but safe)
		if (header.magic != AssetHeader::Magic)
		{
			Terminal::Error(StringOps::GetName(this), "{} Path: {}", "Magic number doesn't match up!", pAsset->m_id.ToString());
			pAsset->m_residency.Store(AssetResidency::Failed, PAL::MemoryOrder::Release);
			PAL::File::ReleaseAccess(fileHandl);
			return;
		}

		// Fail checks (Doesn't happen but safe)
		if (header.version != AssetHeader::Version)
		{
			Terminal::Error(StringOps::GetName(this), "{} Path: {}", "Versions are not matching!", pAsset->m_id.ToString());
			pAsset->m_residency.Store(AssetResidency::Failed, PAL::MemoryOrder::Release);
			PAL::File::ReleaseAccess(fileHandl);
			return;
		}

		if (!PAL::File::ReadMemory(fileHandl, propertyBytes, header.propertyOffset, header.propertyOffset + header.propertySize))
		{
			Terminal::Error(StringOps::GetName(this), "{} Path: {}", "Failed to read memory file for SceneProperties", pAsset->m_id.ToString());
			pAsset->m_residency.Store(AssetResidency::Failed, PAL::MemoryOrder::Release);

			PAL::File::ReleaseAccess(fileHandl);
			return;
		}

		// Decode before reading the dataPayload
		pAsset->m_residency.Store(AssetResidency::Decoding, PAL::MemoryOrder::Relaxed);
		SceneProperties props;
		std::memcpy(&props, propertyBytes.GetData(), sizeof(SceneProperties));

		// If don't do this Archives gives error because there is no data to read.
		if (props.entityCount == 0)
		{
			Terminal::Info(StringOps::GetName(this), "Entity count is 0. Moving on to reduce the errors");
			PAL::File::ReleaseAccess(fileHandl);
			pAsset->m_residency.Store(AssetResidency::Resident, PAL::MemoryOrder::Release);
			return;
		}

		if (!PAL::File::ReadMemory(fileHandl, dataPayload, header.payloadOffset, header.payloadOffset + header.payloadSize))
		{
			Terminal::Error(StringOps::GetName(this), "{} Path: {}", "Failed to read memory file for BinaryData", pAsset->m_id.ToString());
			pAsset->m_residency.Store(AssetResidency::Failed, PAL::MemoryOrder::Release);
			PAL::File::ReleaseAccess(fileHandl);
			return;
		}

		PAL::File::ReleaseAccess(fileHandl);

		pAsset->m_residency.Store(AssetResidency::Uploading, PAL::MemoryOrder::Relaxed);

		{
			ScopedLock lockArena(m_sceneLocker);

			BinaryArchiveReader reader(dataPayload.GetData(), dataPayload.GetCount());
			SceneSerializer::Deserialize(*pAsset->m_scene, GetEngine()->GetReflectionSystem(), reader);
		}

		pAsset->m_residency.Store(AssetResidency::Resident, PAL::MemoryOrder::Release);
	}

	void SceneAssetStreamer::LoadSync(SceneAsset* pAsset)
	{
		if (!pAsset->m_scene)
			pAsset->m_scene = Memory::Allocator::Create<Scene>(Memory::CurrLoc(), GetEngine()->GetReflectionSystem());

		RunLoadWorld(pAsset);
	}

	void SceneAssetStreamer::UnloadSync(SceneAsset* pAsset)
	{
	}
}