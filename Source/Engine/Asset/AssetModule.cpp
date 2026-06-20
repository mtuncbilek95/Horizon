#include "AssetModule.h"

#include <Engine/Asset/Texture/TextureProperties.h>

namespace Horizon
{
	void AssetModule::OnAttach(Engine* pEngine)
	{
		Submodule::OnAttach(pEngine);

		RegisterType<TextureProperties>("Texture");
	}

	void AssetModule::OnSync()
	{
	}

	void AssetModule::OnDetach()
	{
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