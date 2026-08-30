#include "AssetService.h"

#include <Engine/Asset/AssetTypeAttribute.h>
#include <Engine/Asset/AssetLoadStrategy.h>
#include <Engine/Content/ContentContext.h>
#include <Engine/Core/Engine.h>
#include <Engine/Core/ModuleGraph.h>
#include <Engine/Graphics/GraphicsContext.h>
#include <Engine/Reflection/ReflectionSystem.h>

namespace Horizon::Engine
{
	ModuleReport AssetService::OnInitialize()
	{
		auto* pReflect = GetEngine()->GetReflectionSystem();

		List<Reflect::Type*> types = pReflect->GetTypeByAttribute(Reflect::TypeOf<AssetTypeAttribute>());

		for (auto* type : types)
		{
			if (type->GetBaseId() != Reflect::TypeOf<AssetLoadStrategy>())
			{
				Terminal::Warn(StringOps::GetName(this), "{} has not inherited from AssetLoadStrategy! It may a different class " 
					"or you're doing something wrong!", type->GetName());
				continue;
			}

			AssetLoadStrategy* pStrategy = (AssetLoadStrategy*)type->Create();
			m_loaderLookup.emplace(pStrategy->GetWorkingAssetHandle(), m_loaders.GetCount());
			m_loaders.PushBack(pStrategy);

			std::string assetName = pReflect->GetType(pStrategy->GetWorkingAssetHandle())->GetName();
			Terminal::Info(StringOps::GetName(this), "{} has been registered with type {}", type->GetName(), assetName);
		}

		return ModuleReport();
	}

	void AssetService::OnExecute()
	{
	}

	void AssetService::OnFinalize()
	{ 
		for (auto* strategy : m_loaders)
			Memory::Allocator::Delete(strategy);

		m_loaders.Clear();
	}

	void AssetService::DeclareDependencies(ModuleGraph& graph)
	{
		graph.Requires<GraphicsContext>();
		graph.Requires<ContentContext>();
	}

	AssetLoadStrategy* AssetService::FindStrategy(Reflect::TypeHandle assetType)
	{
		auto it = m_loaderLookup.find(assetType);

		if (it == m_loaderLookup.end())
		{
			Terminal::Error(StringOps::GetName(this), "No load strategy is registered for this asset type");
			return nullptr;
		}

		return m_loaders[it->second];
	}
}