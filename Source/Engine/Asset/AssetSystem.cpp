#include "AssetSystem.h"

#include <Engine/Core/Application.h>
#include <Engine/Module/ModuleContext.h>

#include <cstring>

namespace Horizon::Engine
{
	AppReport AssetSystem::OnAttach(Application* pEngine)
	{
		System::OnAttach(pEngine);

		m_typeRegistry.Bootstrap(pEngine);

		return AppReport();
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

	void AssetSystem::RegisterAsset(const AssetEntryDesc& desc)
	{
	}

	void AssetSystem::UnregisterAsset(const Guid& id)
	{
	}
}