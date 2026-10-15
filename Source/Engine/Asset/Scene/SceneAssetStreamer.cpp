#include "SceneAssetStreamer.h"

#include <Engine/Asset/Scene/SceneSerializer.h>
#include <Engine/Asset/Scene/SceneProperties.h>

#include <Runtime/Containers/ScopedLock.h>
#include <Runtime/Containers/StringOps.h>
#include <Runtime/Log/Terminal.h>
#include <Runtime/PAL/File/File.h>
#include <Runtime/Serialization/BinaryArchive.h>

namespace Horizon::Engine
{
	void SceneAssetStreamer::OnInitialize()
	{
		m_jobSystem = GetEngine()->GetJobSystem();
	}


	void SceneAssetStreamer::OnSync(const EngineFrame& frameContext)
	{
	}

	void SceneAssetStreamer::OnFinalize()
	{
	}

	void SceneAssetStreamer::LoadAsync(AssetObject* pAsset)
	{
		auto* pSceneAsset = (SceneAsset*)pAsset;

		if (!pSceneAsset->m_scene)
			pSceneAsset->m_scene = Memory::Allocator::Create<Scene>(Memory::CurrLoc(), GetEngine()->GetReflectionSystem());

		pSceneAsset->m_ticket = m_jobSystem->SubmitJob(JobLane::Background, Job([this, pSceneAsset]()
			{
				RunLoadWorld(pSceneAsset);
			}));

		// Nearly impossible but sure
		if (pSceneAsset->m_ticket == InvalidSubmitTicket)
		{
			Terminal::Error(StringOps::GetName(this), "{} Path: {}", "Job could not be submitted", pSceneAsset->m_ownerEntry.assetId.ToString());
			pSceneAsset->m_residency.Store(AssetResidency::Failed, PAL::MemoryOrder::Release);
		}
	}

	void SceneAssetStreamer::UnloadAsync(AssetObject* pAsset)
	{
	}

	void SceneAssetStreamer::RunLoadWorld(SceneAsset* pAsset)
	{
		const AssetHeader& header = pAsset->m_header;

		// Fail checks (Doesn't happen but safe)
		if (header.magic != AssetHeader::Magic)
		{
			Terminal::Error(StringOps::GetName(this), "{} Path: {}", "Magic number doesn't match up!", pAsset->m_ownerEntry.assetId.ToString());
			pAsset->m_residency.Store(AssetResidency::Failed, PAL::MemoryOrder::Release);
			return;
		}

		// Fail checks (Doesn't happen but safe)
		if (header.version != AssetHeader::Version)
		{
			Terminal::Error(StringOps::GetName(this), "{} Path: {}", "Versions are not matching!", pAsset->m_ownerEntry.assetId.ToString());
			pAsset->m_residency.Store(AssetResidency::Failed, PAL::MemoryOrder::Release);
			return;
		}

		pAsset->m_residency.Store(AssetResidency::Reading, PAL::MemoryOrder::Relaxed);

		List<u8> propertyBytes;
		List<u8> dataPayload;

		// Read payload then put up your ass.
		std::string cookPath = pAsset->m_ownerEntry.cookPath;
		PAL::FileAccessRequest fileHandl = PAL::File::RequestAccess(cookPath, PAL::FileOperationAccessPolicy::Read, PAL::FileOperationSharePolicy::SharedRead);
		if (!fileHandl.IsValid())
		{
			Terminal::Error(StringOps::GetName(this), "{} Path: {}", "Access request to file has failed!", pAsset->m_ownerEntry.assetId.ToString());
			pAsset->m_residency.Store(AssetResidency::Failed, PAL::MemoryOrder::Release);
			return;
		}

		if (!PAL::File::ReadMemory(fileHandl, propertyBytes, header.propertyOffset, header.propertyOffset + header.propertySize))
		{
			Terminal::Error(StringOps::GetName(this), "{} Path: {}", "Failed to read memory file for SceneProperties", pAsset->m_ownerEntry.assetId.ToString());
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
			pAsset->m_residency.Store(AssetResidency::Resident, PAL::MemoryOrder::Relaxed);
			return;
		}

		if (!PAL::File::ReadMemory(fileHandl, dataPayload, header.payloadOffset, header.payloadOffset + header.payloadSize))
		{
			Terminal::Error(StringOps::GetName(this), "{} Path: {}", "Failed to read memory file for BinaryData", pAsset->m_ownerEntry.assetId.ToString());
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

		pAsset->m_residency.Store(AssetResidency::Resident, PAL::MemoryOrder::Relaxed);
	}
}