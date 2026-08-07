#include "AssetSystem.h"

#include <Engine/Core/Engine.h>
#include <Engine/Module/ModuleContext.h>

#include <cstring>

namespace Horizon
{
	EngineReport AssetSystem::OnAttach(Engine* pEngine)
	{
		System::OnAttach(pEngine);

		m_typeRegistry.Bootstrap(pEngine);

		return EngineReport();
	}

	void AssetSystem::OnSync()
	{
		// TODO: free pending load/stream
	}

	void AssetSystem::OnDetach()
	{
	}

	void AssetSystem::GetInitializeOrder(OrderRules& rules) const
	{
	}

	void AssetSystem::GetExecutionOrder(OrderRules& rules) const
	{
	}

	void AssetSystem::RegisterAsset(const AssetRegistryDesc& registerInfo)
	{
	}

	void AssetSystem::UnregisterAsset(const Guid& guid)
	{
	}
}