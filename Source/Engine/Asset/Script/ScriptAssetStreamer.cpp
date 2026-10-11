#include "ScriptAssetStreamer.h"

#include <Engine/Asset/AssetHeader.h>
#include <Engine/Asset/Script/ScriptProperties.h>
#include <Engine/Asset/Script/ScriptSchema.h>
#include <Engine/Reflection/ReflectionSystem.h>

#include <Runtime/Containers/StringOps.h>
#include <Runtime/Log/Terminal.h>
#include <Runtime/PAL/File/File.h>

#include <cstring>
#include <format>

namespace Horizon::Engine
{
	void ScriptAssetStreamer::OnInitialize()
	{
		m_jobSystem = GetEngine()->GetJobSystem();
		m_reflection = GetEngine()->GetReflectionSystem();
	}

	void ScriptAssetStreamer::OnSync(const EngineFrame& frameContext)
	{
	}

	void ScriptAssetStreamer::OnFinalize()
	{
	}

	void ScriptAssetStreamer::LoadAsync(AssetObject* pAsset)
	{
		auto* pScriptAsset = static_cast<ScriptAsset*>(pAsset);

		pScriptAsset->m_ticket = m_jobSystem->SubmitJob(JobLane::Background, Job([this, pScriptAsset]()
			{
				RunLoadAsset(pScriptAsset);
			}));

		if (pScriptAsset->m_ticket == InvalidSubmitTicket)
			FailedAssetLog(pScriptAsset, "Job could not be submitted");
	}

	void ScriptAssetStreamer::UnloadAsync(AssetObject* pAsset)
	{
		auto* pScriptAsset = static_cast<ScriptAsset*>(pAsset);

		pScriptAsset->m_source.clear();
		pScriptAsset->m_chunkName.clear();
		pScriptAsset->m_fields.Clear();

		pScriptAsset->m_residency.Store(AssetResidency::Unloaded, PAL::MemoryOrder::Release);
	}

	void ScriptAssetStreamer::RunLoadAsset(ScriptAsset* pAsset)
	{
		const AssetHeader& header = pAsset->m_header;

		if (header.magic != AssetHeader::Magic)
		{
			FailedAssetLog(pAsset, "Magic number doesn't match up!");
			return;
		}

		if (header.version != AssetHeader::Version)
		{
			FailedAssetLog(pAsset, "Versions are not matching!");
			return;
		}

		if (header.propertySize != sizeof(ScriptProperties))
		{
			FailedAssetLog(pAsset, "ScriptProperties are not matching!");
			return;
		}

		pAsset->m_residency.Store(AssetResidency::Reading, PAL::MemoryOrder::Relaxed);

		std::string cookPath = pAsset->m_ownerEntry.cookPath;
		PAL::FileAccessRequest fileHandl = PAL::File::RequestAccess(cookPath, PAL::FileOperationAccessPolicy::Read, PAL::FileOperationSharePolicy::SharedRead);
		if (!fileHandl.IsValid())
		{
			FailedAssetLog(pAsset, "Access request to file has failed!");
			return;
		}

		PAL::FileView view = PAL::File::OpenMap(fileHandl);
		PAL::File::ReleaseAccess(fileHandl);

		if (!view.IsValid())
		{
			FailedAssetLog(pAsset, "Failed to map the cook file");
			return;
		}

		const usize propertyEnd = header.propertyOffset + header.propertySize;
		const usize payloadEnd = header.payloadOffset + header.payloadSize;
		if (propertyEnd > view.GetSize() || payloadEnd > view.GetSize())
		{
			FailedAssetLog(pAsset, "Header offsets point outside of the cook file");
			PAL::File::CloseMap(view);
			return;
		}

		pAsset->m_residency.Store(AssetResidency::Decoding, PAL::MemoryOrder::Relaxed);

		ScriptProperties props;
		std::memcpy(&props, view.GetData() + header.propertyOffset, sizeof(ScriptProperties));

		if (props.sourceSize == 0 || props.sourceSize != header.payloadSize)
		{
			FailedAssetLog(pAsset, "Script source size does not match the payload");
			PAL::File::CloseMap(view);
			return;
		}

		std::string source(reinterpret_cast<const c8*>(view.GetData() + header.payloadOffset), props.sourceSize);
		PAL::File::CloseMap(view);

		const std::string fileName(props.chunkName, strnlen(props.chunkName, ScriptProperties::MaxChunkName));
		std::string chunkName = std::format("@{}", fileName);

		List<ScriptField> fields;
		std::string error;

		if (!ScriptSchema::Extract(source, chunkName, m_reflection, fields, error))
		{
			FailedAssetLog(pAsset, error);
			return;
		}

		pAsset->m_source = std::move(source);
		pAsset->m_chunkName = std::move(chunkName);
		pAsset->m_fields = std::move(fields);
		pAsset->m_residency.Store(AssetResidency::Resident, PAL::MemoryOrder::Release);
	}
}