#include "AssetModule.h"

#include <Runtime/PAL/Window/Window.h>

#include <Engine/Core/Engine.h>
#include <Engine/Window/WindowModule.h>

#include <Engine/Asset/Texture/TextureProperties.h>

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

	void AssetModule::OnAttach(Engine* pEngine)
	{
		Submodule::OnAttach(pEngine);

		RegisterType<TextureProperties>("Texture");
	}

	void AssetModule::OnSync()
	{
		auto* windowModule = m_engine->TryGetModule<WindowModule>();
		std::span<const InputMessage> messages = windowModule->GetWindow()->GetMessages();

		for (const auto& msg : messages)
		{
			for (const auto& path : msg.filePaths)
				ImportAsset(path, "D:/Projects/Horizon/TestProject/Assets");
		}
	}

	void AssetModule::OnDetach()
	{
	}

	void AssetModule::ImportAsset(const std::filesystem::path& from, const std::filesystem::path& to)
	{
		std::string type = ExtensionToType(from.extension().string());
		if (type.empty())
			return;

		AssetProperties* props = Create(type);
		if (!props)
			return;

		props->Import(from);

		const std::string stem = from.stem().string();
		std::filesystem::path binPath = to / (stem + ".hbin");

		{
			std::ofstream bin(binPath, std::ios::binary);
			props->Cook(bin);
		}

		AssetMetadata metadata = {};
		metadata.assetInfo = props;
		metadata.assetId = Guid::Generate();
		metadata.assetName = stem;
		metadata.assetType = type;
		metadata.binPath = binPath;
		metadata.binarySize = std::filesystem::file_size(binPath);

		metadata.Serialize(to);

		Allocator::Delete(props);

		Terminal::Info("AssetModule", "{} has been serialized", from.string());
	}

	AssetProperties* AssetModule::Create(std::string typeName)
	{
		auto typeIt = m_typeNameToId.find(typeName);
		
		if (typeIt == m_typeNameToId.end())
		{
			Terminal::Error("AssetModule", "{} has not been registered!!!", typeName);
			return nullptr;
		}

		auto assetIt = m_registeredTypes.find(typeIt->second);
		if (assetIt != m_registeredTypes.end())
			return assetIt->second();
		
		Terminal::Error("AssetModule", "Even though asset type {} has been registered, something is wrong", typeName);
		return nullptr;
	}
}