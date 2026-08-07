#include "AssetTypeRegistry.h"

#include <Engine/Asset/AssetTypeAttribute.h>
#include <Engine/Core/Engine.h>
#include <Engine/Module/ModuleContext.h>

#include <Runtime/Log/Terminal.h>

namespace Horizon
{
	void AssetTypeRegistry::Bootstrap(Engine* pEngine)
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