#include "DomainModule.h"

#include <Runtime/PAL/Window/Window.h>

#include <Engine/Core/Engine.h>
#include <Engine/Window/WindowModule.h>

#include <Engine/Asset/AssetMetadata.h>
#include <Engine/Asset/AssetModule.h>

#include <map>
#include <fstream>

namespace Horizon
{
	namespace
	{
		std::string ExtensionToType(const std::string& ext)
		{
			static std::map<std::string, std::string> extMap =
			{
				{".tga", "Texture"}
			};

			auto it = extMap.find(ext);
			if (it != extMap.end())
				return it->second;

			Terminal::Error("AssetTracker", "{} is not a proper extension for this engine", ext);
			return "";
		}
	}

	void DomainModule::OnAttach(Engine* pEngine)
	{
		Submodule::OnAttach(pEngine);
	}

	void DomainModule::OnSync()
	{
		auto* windowModule = m_engine->TryGetModule<WindowModule>();
		auto* window = windowModule->GetWindow();

		for (const auto& msg : window->GetMessages())
		{
			for (const auto& path : msg.filePaths)
				ImportAsset(path, "D:/Projects/Horizon/TestProject/Assets");
		}

	}

	void DomainModule::OnDetach()
	{
	}

	void DomainModule::ImportAsset(const std::filesystem::path& from, const std::filesystem::path& to)
	{
		auto* assetModule = m_engine->TryGetModule<AssetModule>();

		std::string type = ExtensionToType(from.extension().string());
		if (type.empty())
			return;

		AssetProperties* props = assetModule->Create(type);
		if (!props)
			return;

		props->Import(from);

		Guid newGuid = Guid::Generate();
		const std::string stem = from.stem().string();
		std::filesystem::path binPath = to / (newGuid.ToString() + ".hbin");

		{
			std::ofstream bin(binPath, std::ios::binary);
			props->Cook(bin);
		}

		AssetMetadata metadata = {};
		metadata.assetInfo = props;
		metadata.assetId = newGuid;
		metadata.assetName = stem;
		metadata.assetType = type;
		metadata.binPath = binPath;
		metadata.binarySize = std::filesystem::file_size(binPath);

		metadata.Serialize(to);

		Allocator::Delete(props);

		Terminal::Info("AssetModule", "{} has been serialized", from.string());
	}

}