#include "SceneStreamer.h"

#include <Engine/Asset/AssetHeader.h>
#include <Engine/Asset/Scene/SceneSerializer.h>
#include <Engine/Asset/Scene/SceneProperties.h>

#include <Runtime/Containers/ScopedLock.h>
#include <Runtime/Containers/StringHelper.h>
#include <Runtime/Log/Terminal.h>
#include <Runtime/PAL/File/File.h>
#include <Runtime/Serialization/BinaryArchive.h>

namespace Horizon::Asset
{
	void SceneStreamer::LoadAsync(Scene* pAsset)
	{
		if (!pAsset->m_scene)
			pAsset->m_scene = MakeUnique<Engine::Scene>(GetEngine()->GetReflectionSystem());

		pAsset->m_ticket = m_jobSystem->SubmitJob(Engine::JobLane::Background, Engine::Job([this, pAsset]()
			{
				RunLoadWorld(pAsset);
			}));

		// Nearly impossible but sure
		if (pAsset->m_ticket == Engine::InvalidSubmitTicket)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "{} Path: {}", "Engine::Job could not be submitted", pAsset->m_id.ToString());
			pAsset->m_residency.Store(Engine::AssetResidency::Failed, PAL::MemoryOrder::Release);
		}
	}

	void SceneStreamer::UnloadAsync(Scene* pAsset)
	{
	}

	void SceneStreamer::OnInitialize()
	{
		m_jobSystem = GetEngine()->GetJobSystem();
	}

	void SceneStreamer::OnRegisteredAsset(const Engine::AssetEntry& entry)
	{
		auto* pSceneAsset = static_cast<Scene*>(entry.pAssetObject);

		pSceneAsset->m_id = entry.assetId;
		pSceneAsset->m_cookPath = entry.cookPath;
	}

	void SceneStreamer::OnUnregisteredAsset(const Engine::AssetEntry& entry)
	{
		auto* pSceneAsset = static_cast<Scene*>(entry.pAssetObject);

		if (pSceneAsset->m_ticket != Engine::InvalidSubmitTicket)
			m_jobSystem->WaitTicket(pSceneAsset->m_ticket);
	}

	void SceneStreamer::OnRefreshAsset(const Engine::AssetEntry& entry)
	{
		auto* pSceneAsset = static_cast<Scene*>(entry.pAssetObject);

		pSceneAsset->m_residency.CompareExchange(Engine::AssetResidency::Failed, Engine::AssetResidency::Unloaded);
	}

	void SceneStreamer::RunLoadWorld(Scene* pAsset)
	{
		pAsset->m_residency.Store(Engine::AssetResidency::Reading, PAL::MemoryOrder::Relaxed);

		List<u8> headerBytes;
		List<u8> propertyBytes;
		List<u8> dataPayload;

		// Read payload then put up your ass.
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

		// Fail checks (Doesn't happen but safe)
		if (header.magic != Engine::AssetHeader::Magic)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "{} Path: {}", "Magic number doesn't match up!", pAsset->m_id.ToString());
			pAsset->m_residency.Store(Engine::AssetResidency::Failed, PAL::MemoryOrder::Release);
			PAL::File::ReleaseAccess(fileHandl);
			return;
		}

		// Fail checks (Doesn't happen but safe)
		if (header.version != Engine::AssetHeader::Version)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "{} Path: {}", "Versions are not matching!", pAsset->m_id.ToString());
			pAsset->m_residency.Store(Engine::AssetResidency::Failed, PAL::MemoryOrder::Release);
			PAL::File::ReleaseAccess(fileHandl);
			return;
		}

		if (!PAL::File::ReadMemory(fileHandl, propertyBytes, header.propertyOffset, header.propertyOffset + header.propertySize))
		{
			Terminal::Error(StringHelper::GetTypeName(this), "{} Path: {}", "Failed to read memory file for SceneProperties", pAsset->m_id.ToString());
			pAsset->m_residency.Store(Engine::AssetResidency::Failed, PAL::MemoryOrder::Release);

			PAL::File::ReleaseAccess(fileHandl);
			return;
		}

		// Decode before reading the dataPayload
		pAsset->m_residency.Store(Engine::AssetResidency::Decoding, PAL::MemoryOrder::Relaxed);
		SceneProperties props;
		std::memcpy(&props, propertyBytes.GetData(), sizeof(SceneProperties));

		// If don't do this Archives gives error because there is no data to read.
		if (props.entityCount == 0)
		{
			Terminal::Info(StringHelper::GetTypeName(this), "Entity count is 0. Moving on to reduce the errors");
			PAL::File::ReleaseAccess(fileHandl);
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

		pAsset->m_residency.Store(Engine::AssetResidency::Uploading, PAL::MemoryOrder::Relaxed);

		{
			ScopedLock lockArena(m_sceneLocker);

			BinaryArchiveReader reader(dataPayload.GetData(), dataPayload.GetCount());
			SceneSerializer::Deserialize(*pAsset->m_scene, GetEngine()->GetReflectionSystem(), reader);
		}

		pAsset->m_residency.Store(Engine::AssetResidency::Resident, PAL::MemoryOrder::Release);
	}

	void SceneStreamer::LoadSync(Scene* pAsset)
	{
		if (!pAsset->m_scene)
			pAsset->m_scene = MakeUnique<Engine::Scene>(GetEngine()->GetReflectionSystem());

		RunLoadWorld(pAsset);
	}

	void SceneStreamer::UnloadSync(Scene* pAsset)
	{
	}
}