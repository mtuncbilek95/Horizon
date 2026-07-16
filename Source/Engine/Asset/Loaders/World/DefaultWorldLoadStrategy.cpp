#include "DefaultWorldLoadStrategy.h"

#include <Engine/Asset/Loaders/World/WorldDescriptor.h>
#include <Engine/Asset/Loaders/World/WorldAsset.h>
#include <Engine/Asset/AssetEntry.h>
#include <Engine/ECS/WorldRegistry.h>
#include <Engine/ECS/EntityRegistry.h>
#include <Engine/ECS/ComponentRegistry.h>

#include <Runtime/PAL/File/File.h>
#include <Runtime/PAL/File/FileAccessRequest.h>

#include <nlohmann/json.hpp>
#include <filesystem>
#include <string>
#include <string_view>

namespace Horizon
{
	IAsset* DefaultWorldLoadStrategy::Load(const AssetEntry& entry)
	{
		PAL::FileAccessRequest req = PAL::File::RequestAccess(entry.GetCookedPath(),
			PAL::FileOperationAccessPolicy::Read, PAL::FileOperationSharePolicy::SharedRead);

		if (!req.IsValid())
			return nullptr;

		std::vector<u8> descBytes;
		b8 ok = PAL::File::ReadMemory(req, descBytes,
			entry.GetDescriptorOffset(), entry.GetDescriptorOffset() + entry.GetDescriptorSize());

		std::vector<u8> payloadBytes;
		if (ok)
			ok = PAL::File::ReadMemory(req, payloadBytes,
				entry.GetPayloadOffset(), entry.GetPayloadOffset() + entry.GetPayloadSize());

		PAL::File::ReleaseAccess(req);

		if (!ok)
			return nullptr;

		WorldDescriptor descriptor;
		if (!descriptor.Deserialize(descBytes.data(), descBytes.size()))
		{
			Terminal::Warn("WorldLoadStrategy", "Bad descriptor: {}", entry.GetCookedPath().string());
			return nullptr;
		}

		auto j = nlohmann::json::parse(std::string((const c8*)payloadBytes.data(), payloadBytes.size()), nullptr, false);
		if (j.is_discarded())
		{
			Terminal::Warn("WorldLoadStrategy", "Bad payload: {}", entry.GetCookedPath().string());
			return nullptr;
		}

		WorldAsset* world = Allocator::Create<WorldAsset>(CurrLoc());
		world->SetWorldName(descriptor.GetWorldName());

		for (const auto& e : j.value("entities", nlohmann::json::array()))
		{
			EntityHandle handle = world->GetRegistry().CreateEntity();

			std::string name = e.value("name", std::string{});
			if (!name.empty())
				world->SetName(handle, name);

			// TODO: Handle Components
		}

		Terminal::Log("WorldLoadStrategy", "Loaded world '{}'", descriptor.GetWorldName());
		return world;
	}
}