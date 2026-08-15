#include "AssetTypeRegistry.h"

#include <Engine/Asset/AssetObject.h>
#include <Engine/Asset/AssetTypeAttribute.h>
#include <Engine/Core/Application.h>
#include <Engine/Module/ModuleContext.h>

#include <Runtime/Log/Terminal.h>

namespace Horizon::Engine
{
	void AssetTypeRegistry::Bootstrap(Application* pEngine)
	{
		auto* pMod = pEngine->GetModuleContext();

		List<Reflect::Type*> types = pMod->GetTypeByAttribute(Reflect::TypeOf<AssetTypeAttribute>());

		for (auto* type : types)
		{
			if (type->GetBaseId() != Reflect::TypeOf<AssetObject>())
			{
				Terminal::Error("AssetTypeRegistry", "If you're seeing this, your asset does not inherited from AssetObject -> {}", type->GetName());
				continue;
			}

			auto* pAttr = type->GetCustomAttribute<AssetTypeAttribute>();
			m_registryAuxiliary[type->GetTypeId()] = m_registries.GetCount();
			m_registries.EmplaceBack(type, pAttr->GetTypeName(), pAttr->GetVersion(), pAttr->GetOrigin());
		}
	}

	const AssetTypeDesc& AssetTypeRegistry::GetAssetDescriptor(Reflect::TypeHandle handl)
	{
		auto it = m_registryAuxiliary.find(handl);
		if (it == m_registryAuxiliary.end())
			throw std::runtime_error("Missing asset auxiliary somehow");

		return m_registries[it->second];
	}
}