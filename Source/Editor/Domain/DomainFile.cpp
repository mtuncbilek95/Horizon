#include "DomainFile.h"

#include <Engine/Core/Engine.h>
#include <Engine/Asset/AssetSystem.h>
#include <Runtime/PAL/File/File.h>

#include <nlohmann/json.hpp>

namespace Horizon
{
	DomainFile::DomainFile(const DomainFileDesc& desc, Engine* pEngine)
		: m_engine(pEngine), m_parent(desc.pParent), m_metaPath(desc.metaPath)
	{
		m_name = m_metaPath.stem().string();

		m_valid = LoadMeta();

		auto& assetSub = pEngine->GetSystem<AssetSystem>();
		assetSub.RegisterAsset(m_id, m_sourcePath);
	}

	DomainFile::~DomainFile()
	{
		auto& assetSub = m_engine->GetSystem<AssetSystem>();
		assetSub.UnregisterAsset(m_id);
	}

	b8 DomainFile::LoadMeta()
	{
		auto req = PAL::File::RequestAccess(m_metaPath,
			PAL::FileOperationAccessPolicy::Read, PAL::FileOperationSharePolicy::SharedRead);

		if (!req.IsValid())
			return false;

		std::vector<u8> bytes;
		b8 ok = PAL::File::ReadMemory(req, bytes, 0, 0);
		PAL::File::ReleaseAccess(req);

		if (!ok || bytes.empty())
		{
			Terminal::Warn("DomainFile", "Empty or unreadable meta: {}", m_metaPath.string());
			return false;
		}

		auto j = nlohmann::json::parse(std::string((const c8*)bytes.data(), bytes.size()), nullptr, false);
		if (j.is_discarded())
		{
			Terminal::Warn("DomainFile", "Malformed meta: {}", m_metaPath.string());
			return false;
		}

		std::string guidStr = j.value("guid", std::string{});
		if (guidStr.empty())
		{
			Terminal::Warn("DomainFile", "Meta has no guid: {}", m_metaPath.string());
			return false;
		}

		m_id = Guid(guidStr);

		std::string source = j.value("source", std::string{});
		if (!source.empty())
			m_sourcePath = source;

		return true;
	}
}