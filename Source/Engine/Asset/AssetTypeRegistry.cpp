#include "AssetTypeRegistry.h"

#include <Engine/Asset/AssetTypeAttribute.h>
#include <Engine/Core/Application.h>
#include <Engine/Module/ModuleContext.h>

#include <Runtime/Log/Terminal.h>

namespace Horizon::Engine
{
	void AssetTypeRegistry::Bootstrap(Application* pEngine)
	{
	}

	const AssetTypeDesc* AssetTypeRegistry::ResolveByExtension(const std::string& ext) const
	{
		auto it = m_byExtension.find(ext);

		if (it == m_byExtension.end())
		{
			Terminal::Warn("AssetTypeRegistry", "{} has no asset type", ext);
			return nullptr;
		}

		return it->second;
	}
}