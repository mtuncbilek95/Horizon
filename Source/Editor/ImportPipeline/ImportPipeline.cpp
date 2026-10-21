#include "ImportPipeline.h"

#include <Engine/Asset/AssetHeader.h>
#include <Runtime/Containers/List.h>
#include <Runtime/Containers/StringOps.h>
#include <Runtime/Log/Terminal.h>
#include <Runtime/PAL/File/Directory.h>
#include <Runtime/PAL/File/File.h>

namespace Horizon::Editor
{
	AssetImportResult ImportPipeline::Cook(const ImportDeclaredProduct& product, const ImportPipelineContext& ctx, const Guid& id, 
		const Reflect::Type* pAssetType, const std::string& cookedPath, ImportProductBytes& bytes)
	{
		PostProcess(product, ctx, bytes);

		const AssetImportResult result = Validate(product, bytes);
		if (result != AssetImportResult::Success)
			return result;

		if (!Write(cookedPath, id, pAssetType, bytes))
			return AssetImportResult::SerializationFailed;

		return AssetImportResult::Success;
	}

	b8 ImportPipeline::Write(const std::string& cookedPath, const Guid& id, const Reflect::Type* pAssetType, const ImportProductBytes& bytes)
	{
		Engine::AssetHeader header = {};
		header.magic = Engine::AssetHeader::Magic;
		header.version = Engine::AssetHeader::Version;
		header.id = id;

		StringOps::CopyToChar(header.typeName, pAssetType->GetName());
		header.propertyOffset = sizeof(Engine::AssetHeader);
		header.propertySize = bytes.properties.GetCount();
		header.tableOffset = header.propertyOffset + header.propertySize;
		header.tableSize = bytes.table.GetCount();
		header.payloadOffset = header.tableOffset + header.tableSize;
		header.payloadSize = bytes.binary.GetCount();

		List<u8> file(header.payloadOffset + header.payloadSize);
		std::memcpy(file.GetData(), &header, sizeof(Engine::AssetHeader));

		// Boring memcpy, if anyone knows a solution, hit me!
		if (header.propertySize > 0)
			std::memcpy(file.GetData() + header.propertyOffset, bytes.properties.GetData(), header.propertySize);

		if (header.tableSize > 0)
			std::memcpy(file.GetData() + header.tableOffset, bytes.table.GetData(), header.tableSize);

		if (header.payloadSize > 0)
			std::memcpy(file.GetData() + header.payloadOffset, bytes.binary.GetData(), header.payloadSize);

		const std::string folder = StringOps::ParentPathOf(cookedPath);
		if (!PAL::Directory::Exists(folder) && !PAL::Directory::Create(folder))
		{
			Terminal::Error(StringOps::GetName(this), "{} cook folder could not be created", folder);
			return false;
		}

		if (!PAL::File::Exists(cookedPath) && !PAL::File::Create(cookedPath))
		{
			Terminal::Error(StringOps::GetName(this), "{} could not be created", cookedPath);
			return false;
		}

		PAL::FileAccessRequest request = PAL::File::RequestAccess(cookedPath, PAL::FileOperationAccessPolicy::Write, PAL::FileOperationSharePolicy::Exclusive);
		if (!request.IsValid())
		{
			Terminal::Error(StringOps::GetName(this), "{} could not be opened for writing", cookedPath);
			return false;
		}

		const b8 wasWritten = PAL::File::Truncate(request, 0) && PAL::File::WriteMemory(request, file);
		PAL::File::ReleaseAccess(request);

		if (!wasWritten)
			Terminal::Error(StringOps::GetName(this), "{} could not be written", cookedPath);

		return wasWritten;
	}
}