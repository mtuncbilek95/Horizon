#include "WorldAssetImporter.h"

#include <Editor/Domain/Importer/AssetImportContext.h>
#include <Engine/Asset/Loaders/World/WorldDescriptor.h>
#include <Engine/Asset/AssetHeader.h>
#include <Runtime/PAL/File/File.h>

#include <nlohmann/json.hpp>
#include <cstring>

namespace Horizon
{
	namespace
	{
		b8 WriteWholeFile(const std::filesystem::path& path, const std::vector<u8>& bytes)
		{
			auto req = PAL::File::RequestAccess(path,
				PAL::FileOperationAccessPolicy::Write, PAL::FileOperationSharePolicy::Exclusive);

			if (!req.IsValid())
				return false;

			b8 ok = PAL::File::WriteMemory(req, bytes, 0);
			PAL::File::ReleaseAccess(req);
			return ok;
		}
	}

	void WorldAssetImporter::OnImport(AssetImportContext& context)
	{
	}

	void WorldAssetImporter::OnImportDefault(AssetImportContext& context)
	{
		const Guid& guid = context.AssetGuid();
		std::string assetName = context.GetMetaPath().stem().string();

		nlohmann::json meta;
		meta["guid"] = guid.ToString();
		meta["name"] = assetName;
		meta["source"] = context.GetCookPath();
		meta["info"] = nlohmann::json::object();

		std::string metaText = meta.dump(2);
		std::vector<u8> metaBytes(metaText.begin(), metaText.end());

		if (!WriteWholeFile(context.GetMetaPath(), metaBytes))
		{
			Terminal::Error("WorldAssetImporter", "Failed to write meta {}", context.GetMetaPath().string());
			return;
		}

		WorldDescriptor descriptor(assetName, 0);

		std::vector<u8> descBytes;
		descriptor.Serialize(descBytes);

		std::string payloadText = R"({"entities":[]})";
		std::vector<u8> payloadBytes(payloadText.begin(), payloadText.end());

		AssetHeader header;
		header.guid = guid;
		header.SetType("World");

		header.depsCount = 0;
		header.depsOffset = sizeof(AssetHeader);
		header.descriptorOffset = header.depsOffset;
		header.descriptorSize = descBytes.size();
		header.payloadOffset = header.descriptorOffset + header.descriptorSize;
		header.payloadSize = payloadBytes.size();

		std::vector<u8> cooked(sizeof(AssetHeader));
		std::memcpy(cooked.data(), &header, sizeof(AssetHeader));

		cooked.insert(cooked.end(), descBytes.begin(), descBytes.end());
		cooked.insert(cooked.end(), payloadBytes.begin(), payloadBytes.end());

		if (!WriteWholeFile(context.GetCookPath(), cooked))
		{
			Terminal::Error("WorldAssetImporter", "Failed to write cooked {}", context.GetCookPath().string());
			return;
		}

		Terminal::Log("WorldAssetImporter", "Default world '{}' -> {} ({} bytes)",
			assetName, context.GetCookPath().string(), cooked.size());
	}
}