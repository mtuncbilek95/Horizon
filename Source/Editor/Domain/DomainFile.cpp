// Editor/Domain/DomainFile.cpp
#include "DomainFile.h"

#include <Runtime/PAL/File/File.h>
#include <Runtime/Log/Terminal.h>

#include <nlohmann/json.hpp>

namespace Horizon
{
	DomainFile::DomainFile(const DomainFileDesc& desc, Engine* pEngine)
		: m_engine(pEngine), m_parentFolder(desc.parentFolder),
		m_metaPath(desc.metaPath), m_name(desc.name)
	{
		m_valid = LoadMeta();
	}

	DomainFile::~DomainFile()
	{
		// TODO: This should be finished up — AssetSystem::Unregister?
	}

	b8 DomainFile::LoadMeta()
	{
		auto req = PAL::File::RequestAccess(m_metaPath,
			PAL::FileOperationAccessPolicy::Read, PAL::FileOperationSharePolicy::SharedRead);
		if (!req.IsValid())
		{
			Terminal::Warn("DomainFile", "Cannot open meta: {}", m_metaPath.string());
			return false;
		}

		std::vector<u8> bytes;
		b8 ok = PAL::File::ReadMemory(req, bytes, 0, 0);
		PAL::File::ReleaseAccess(req);

		if (!ok || bytes.empty())
			return false;

		auto json = nlohmann::json::parse(std::string((const c8*)bytes.data(), bytes.size()), nullptr, false);
		if (json.is_discarded())
		{
			Terminal::Warn("DomainFile", "Malformed meta: {}", m_metaPath.string());
			return false;
		}

		std::string source = json.value("source", std::string{});
		if (!source.empty())
			m_sourcePath = source;

		m_assets.clear();
		if (json.contains("assets"))
		{
			for (const auto& entry : json["assets"])
			{
				DomainAsset asset;
				asset.subName = entry.value("sub", std::string{});
				asset.assetTypeName = entry.value("type", std::string{ "Unknown" });
				asset.guid = Guid(entry.value("guid", std::string{}));
				m_assets.push_back(std::move(asset));
			}
		}

		// TODO: This should be finished up — import settings
		return true;
	}
}